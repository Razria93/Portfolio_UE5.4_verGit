#include "Component/CExecutionCollaborationComponent.h"

#include "Action/CAction.h"
#include "Component/CActionComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CStateComponent.h"
#include "Reaction/CReaction.h"
#include "Type/CActionDataTypes.h"
#include "Type/CActionOrchestrationTypes.h"
#include "Type/CCombatTargetTypes.h"
#include "Type/CReactionOrchestrationTypes.h"

#include "GameFramework/Character.h"

UCExecutionCollaborationComponent::UCExecutionCollaborationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Component Reference

void UCExecutionCollaborationComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	HealthComp_Injected = InReferences.HealthComponent;
	StateComp_Injected = InReferences.StateComponent;
	BalanceComp_Injected = InReferences.BalanceComponent;
	CombatTargetComp_Injected = InReferences.CombatTargetComponent;
	CombatSignalTargetComp_Injected = InReferences.CombatSignalTargetComponent;
	ActionOrchestratorComp_Injected = InReferences.ActionOrchestratorComponent;
	ReactionOrchestratorComp_Injected = InReferences.ReactionOrchestratorComponent;
	ActionComp_Injected = InReferences.ActionComponent;
	ReactionComp_Injected = InReferences.ReactionComponent;

	if (IsValid(HealthComp_Injected))
	{
		HealthComp_Injected->OnDeadStateChanged.RemoveAll(this);
		HealthComp_Injected->OnDeadStateChanged.AddUObject(this, &UCExecutionCollaborationComponent::HandleDeadStateChanged);
	}

	if (IsValid(CombatTargetComp_Injected))
	{
		CombatTargetComp_Injected->OnCombatTargetChanged.RemoveAll(this);
		CombatTargetComp_Injected->OnCombatTargetChanged.AddUObject(this, &UCExecutionCollaborationComponent::HandleCombatTargetChanged);
	}

	if (IsValid(ActionComp_Injected))
	{
		ActionComp_Injected->OnActionEvent.RemoveAll(this);
		ActionComp_Injected->OnActionEvent.AddDynamic(this, &UCExecutionCollaborationComponent::HandleActionEvent);
	}

	if (IsValid(ReactionComp_Injected))
	{
		ReactionComp_Injected->OnReactionExecutionLifecycleEvent.RemoveAll(this);
		ReactionComp_Injected->OnReactionExecutionLifecycleEvent.AddUObject(this, &UCExecutionCollaborationComponent::HandleReactionExecutionLifecycleEvent);
	}
}

// Lifecycle

void UCExecutionCollaborationComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	CancelActiveExecutionSession(EExecutionCollaborationCancelReason::ParticipantEndPlay, true);

	if (IsValid(HealthComp_Injected)) HealthComp_Injected->OnDeadStateChanged.RemoveAll(this);
	if (IsValid(CombatTargetComp_Injected)) CombatTargetComp_Injected->OnCombatTargetChanged.RemoveAll(this);
	if (IsValid(ActionComp_Injected)) ActionComp_Injected->OnActionEvent.RemoveAll(this);
	if (IsValid(ReactionComp_Injected)) ReactionComp_Injected->OnReactionExecutionLifecycleEvent.RemoveAll(this);

	Super::EndPlay(InEndPlayReason);
}

// Source Request

bool UCExecutionCollaborationComponent::RequestExecutionForCurrentTarget(const EExecutionOutcomePolicy InOutcomePolicy)
{
	if (HasActiveExecutionSession() || !CanStartSourceExecution()) return false;
	if (InOutcomePolicy == EExecutionOutcomePolicy::None || InOutcomePolicy == EExecutionOutcomePolicy::Max) return false;
	if (!IsValid(CombatTargetComp_Injected) || !IsValid(ActionOrchestratorComp_Injected)) return false;

	const FCombatTargetSnapshot targetSnapshot = CombatTargetComp_Injected->GetCombatTargetSnapshot();
	if (!IsValid(targetSnapshot.TargetActor) || targetSnapshot.Revision <= 0) return false;
	UCExecutionCollaborationComponent* targetCollaborationComp = targetSnapshot.TargetActor->FindComponentByClass<UCExecutionCollaborationComponent>();
	if (!IsValid(targetCollaborationComp)) return false;

	FExecutionSessionId sessionId;
	sessionId.SourceActor = OwnerCharacter_Injected;
	sessionId.Serial = AllocateSessionSerial();

	FExecutionCollaborationContext context;
	if (!targetCollaborationComp->AcceptExecutionReservation(sessionId, OwnerCharacter_Injected, targetSnapshot.Revision, InOutcomePolicy, context)) return false;

	ActiveContext = context;
	CollaborationState = EExecutionCollaborationState::Reserved;
	bIsSourceRole = true;
	bSourceActionTerminal = false;
	bTargetReactionTerminal = false;

	if (!StartTargetExecutionReaction())
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::TargetReactionRejected, true);
		return false;
	}

	if (!StartSourceExecutionAction())
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::SourceActionRejected, true);
		return false;
	}

	CollaborationState = EExecutionCollaborationState::Active;
	return true;
}

// Source Commit

bool UCExecutionCollaborationComponent::HandleSourceExecutionCommit(const uint32 InActionRequestSerial)
{
	if (!bIsSourceRole || CollaborationState != EExecutionCollaborationState::Active) return false;
	if (InActionRequestSerial == 0 || InActionRequestSerial != ActiveContext.SessionId.Serial) return false;
	if (!IsSourceTargetSnapshotCurrent())
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::TargetChanged, true);
		return false;
	}
	if (!IsTargetExecutionOpportunityCurrent())
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::BalanceOpportunityInvalidated, true);
		return false;
	}

	UCExecutionCollaborationComponent* targetCollaborationComp = FindPartnerCollaborationComponent();
	UCCombatSignalTargetComponent* targetSignalComp = IsValid(targetCollaborationComp) ? targetCollaborationComp->CombatSignalTargetComp_Injected : nullptr;
	if (!IsValid(targetSignalComp))
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::InvalidParticipant, true);
		return false;
	}

	FExecutionOutcomePacket outcomePacket;
	outcomePacket.CollaborationContext = ActiveContext;
	if (!targetSignalComp->RequestExecutionOutcomeTarget(outcomePacket))
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::BalanceOpportunityInvalidated, true);
		return false;
	}

	if (IsActiveSession(outcomePacket.CollaborationContext.SessionId))
	{
		CollaborationState = EExecutionCollaborationState::Committed;
	}
	return true;
}

// Target Outcome

bool UCExecutionCollaborationComponent::CommitExecutionOutcome(const FExecutionOutcomePacket& InPacket)
{
	const FExecutionCollaborationContext& context = InPacket.CollaborationContext;
	if (!IsActiveSession(context.SessionId) || bIsSourceRole || !context.IsValidMinimal()) return false;
	if (CollaborationState == EExecutionCollaborationState::Committed) return false;
	if (!ActiveContext.OpportunityReservation.Matches(context.OpportunityReservation)
		|| ActiveContext.OutcomePolicy != context.OutcomePolicy)
	{
		return false;
	}

	CollaborationState = EExecutionCollaborationState::Committed;
	if (!IsValid(BalanceComp_Injected) || !BalanceComp_Injected->ConsumeExecutionOpportunityReservation(context.OpportunityReservation))
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::BalanceOpportunityInvalidated, true);
		return false;
	}

	if (UCExecutionCollaborationComponent* sourceCollaborationComp = FindPartnerCollaborationComponent())
	{
		sourceCollaborationComp->ReceivePartnerCommit(context.SessionId);
	}
	return true;
}

// Query

bool UCExecutionCollaborationComponent::HasActiveExecutionSession() const
{
	return ActiveContext.IsValidMinimal()
		&& CollaborationState != EExecutionCollaborationState::None;
}

// Participant Event Observation

void UCExecutionCollaborationComponent::HandleActionEvent(ACharacter* InOwnerCharacter, const EActionType InActionType, const int32 InActionIndex, const uint32 InActionRequestSerial, const EActionEventType InActionEventType)
{
	if (!bIsSourceRole || !HasActiveExecutionSession()) return;
	if (InOwnerCharacter != OwnerCharacter_Injected || InActionType != EActionType::Execution || InActionRequestSerial != ActiveContext.SessionId.Serial) return;

	if (InActionEventType == EActionEventType::ActionCompleted)
	{
		if (CollaborationState != EExecutionCollaborationState::Committed)
		{
			CancelActiveExecutionSession(EExecutionCollaborationCancelReason::SourceActionInterrupted, true);
			return;
		}

		bSourceActionTerminal = true;
		if (UCExecutionCollaborationComponent* targetCollaborationComp = FindPartnerCollaborationComponent())
		{
			targetCollaborationComp->ReceivePartnerSourceActionTerminal(ActiveContext.SessionId);
		}
		TryCompleteActiveExecutionSession();
		return;
	}

	if (InActionEventType == EActionEventType::ActionInterrupted || InActionEventType == EActionEventType::ActionIgnored)
	{
		if (CollaborationState == EExecutionCollaborationState::Committed)
		{
			bSourceActionTerminal = true;
			if (UCExecutionCollaborationComponent* targetCollaborationComp = FindPartnerCollaborationComponent())
			{
				targetCollaborationComp->ReceivePartnerSourceActionTerminal(ActiveContext.SessionId);
			}
			TryCompleteActiveExecutionSession();
			return;
		}

		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::SourceActionInterrupted, true);
	}
}

void UCExecutionCollaborationComponent::HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent)
{
	if (bIsSourceRole || !HasActiveExecutionSession()) return;
	if (InEvent.Context.ReactionDataKey.ReactionType != EReactionType::Execution) return;

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed)
	{
		if (CollaborationState != EExecutionCollaborationState::Committed)
		{
			CancelActiveExecutionSession(EExecutionCollaborationCancelReason::TargetReactionInterrupted, true);
			return;
		}

		bTargetReactionTerminal = true;
		if (UCExecutionCollaborationComponent* sourceCollaborationComp = FindPartnerCollaborationComponent())
		{
			sourceCollaborationComp->ReceivePartnerTargetReactionTerminal(ActiveContext.SessionId);
		}
		TryCompleteActiveExecutionSession();
		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Interrupted || InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored)
	{
		if (CollaborationState == EExecutionCollaborationState::Committed)
		{
			bTargetReactionTerminal = true;
			if (UCExecutionCollaborationComponent* sourceCollaborationComp = FindPartnerCollaborationComponent())
			{
				sourceCollaborationComp->ReceivePartnerTargetReactionTerminal(ActiveContext.SessionId);
			}
			TryCompleteActiveExecutionSession();
			return;
		}

		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::TargetReactionInterrupted, true);
	}
}

void UCExecutionCollaborationComponent::HandleDeadStateChanged(const EDeadState InPreviousState, const EDeadState InNewState)
{
	if (InNewState != EDeadState::Alive)
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::ParticipantDeath, true);
	}
}

void UCExecutionCollaborationComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	if (bIsSourceRole
		&& CollaborationState != EExecutionCollaborationState::Committed
		&& HasActiveExecutionSession()
		&& !IsSourceTargetSnapshotCurrent())
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::TargetChanged, true);
	}
}

// Partner Coordination

bool UCExecutionCollaborationComponent::AcceptExecutionReservation(const FExecutionSessionId& InSessionId, AActor* InSourceActor, const int32 InSourceTargetRevision, const EExecutionOutcomePolicy InOutcomePolicy, FExecutionCollaborationContext& OutContext)
{
	OutContext = FExecutionCollaborationContext();
	if (HasActiveExecutionSession() || !InSessionId.IsValidMinimal() || !IsValid(InSourceActor) || InSourceTargetRevision <= 0) return false;
	if (!CanStartTargetExecution()) return false;

	FExecutionOpportunityReservation reservation;
	if (!BalanceComp_Injected->TryReserveExecutionOpportunity(InSessionId, reservation)) return false;

	FExecutionCollaborationContext context;
	context.SessionId = InSessionId;
	context.TargetActor = OwnerCharacter_Injected;
	context.SourceTargetRevision = InSourceTargetRevision;
	context.OpportunityReservation = reservation;
	context.OutcomePolicy = InOutcomePolicy;

	ActiveContext = context;
	CollaborationState = EExecutionCollaborationState::Reserved;
	bIsSourceRole = false;
	bSourceActionTerminal = false;
	bTargetReactionTerminal = false;
	OutContext = context;
	return true;
}

void UCExecutionCollaborationComponent::ReceivePartnerSourceActionTerminal(const FExecutionSessionId& InSessionId)
{
	if (!IsActiveSession(InSessionId)) return;
	bSourceActionTerminal = true;
	TryCompleteActiveExecutionSession();
}

void UCExecutionCollaborationComponent::ReceivePartnerTargetReactionTerminal(const FExecutionSessionId& InSessionId)
{
	if (!IsActiveSession(InSessionId)) return;
	bTargetReactionTerminal = true;
	TryCompleteActiveExecutionSession();
}

void UCExecutionCollaborationComponent::ReceivePartnerCommit(const FExecutionSessionId& InSessionId)
{
	if (!IsActiveSession(InSessionId)) return;
	CollaborationState = EExecutionCollaborationState::Committed;
}

void UCExecutionCollaborationComponent::ReceivePartnerCancellation(const FExecutionSessionId& InSessionId, const EExecutionCollaborationCancelReason InReason)
{
	if (!IsActiveSession(InSessionId)) return;
	CancelActiveExecutionSession(InReason, false);
}

// Session Control

bool UCExecutionCollaborationComponent::StartTargetExecutionReaction()
{
	UCExecutionCollaborationComponent* targetCollaborationComp = FindPartnerCollaborationComponent();
	if (!IsValid(targetCollaborationComp) || !IsValid(targetCollaborationComp->ReactionOrchestratorComp_Injected)) return false;

	FExecutionReactionRequest request;
	request.CollaborationContext = ActiveContext;
	const FReactionRequestResult result = targetCollaborationComp->ReactionOrchestratorComp_Injected->RequestExecutionReaction(request);
	if (!result.IsAccepted()) return false;

	targetCollaborationComp->CollaborationState = EExecutionCollaborationState::Starting;
	return true;
}

bool UCExecutionCollaborationComponent::StartSourceExecutionAction()
{
	if (!IsValid(ActionOrchestratorComp_Injected)) return false;

	FExecutionActionRequest request;
	request.CollaborationContext = ActiveContext;
	const FActionRequestResult result = ActionOrchestratorComp_Injected->RequestExecutionAction(request);
	return result.IsAccepted();
}

void UCExecutionCollaborationComponent::CancelActiveExecutionSession(const EExecutionCollaborationCancelReason InReason, const bool bNotifyPartner)
{
	if (!HasActiveExecutionSession()) return;

	const FExecutionCollaborationContext context = ActiveContext;
	const FExecutionSessionId sessionId = context.SessionId;
	const bool bWasSourceRole = bIsSourceRole;
	UCExecutionCollaborationComponent* partnerCollaborationComp = bNotifyPartner ? FindPartnerCollaborationComponent() : nullptr;

	ResetActiveExecutionSession();

	if (!bWasSourceRole && IsValid(BalanceComp_Injected))
	{
		BalanceComp_Injected->ReleaseExecutionOpportunityReservation(context.OpportunityReservation);
	}

	CancelLocalExecutionParticipant(bWasSourceRole);

	if (IsValid(partnerCollaborationComp))
	{
		partnerCollaborationComp->ReceivePartnerCancellation(sessionId, InReason);
	}
}

void UCExecutionCollaborationComponent::CompleteActiveExecutionSession()
{
	if (!HasActiveExecutionSession()) return;
	ResetActiveExecutionSession();
}

void UCExecutionCollaborationComponent::TryCompleteActiveExecutionSession()
{
	if (CollaborationState != EExecutionCollaborationState::Committed) return;
	if (!bSourceActionTerminal || !bTargetReactionTerminal) return;
	CompleteActiveExecutionSession();
}

void UCExecutionCollaborationComponent::CancelLocalExecutionParticipant(const bool bWasSourceRole)
{
	if (bWasSourceRole)
	{
		if (IsValid(ActionComp_Injected) && ActionComp_Injected->IsActiveActionType(EActionType::Execution))
		{
			ActionComp_Injected->CancelActiveActionForSystem();
		}
		return;
	}

	if (IsValid(ReactionComp_Injected) && ReactionComp_Injected->IsActiveReactionType(EReactionType::Execution))
	{
		ReactionComp_Injected->CancelActiveReactionForSystem();
	}
}

// Session Validation

bool UCExecutionCollaborationComponent::IsActiveSession(const FExecutionSessionId& InSessionId) const
{
	return HasActiveExecutionSession() && ActiveContext.SessionId == InSessionId;
}

bool UCExecutionCollaborationComponent::CanStartSourceExecution()
{
	if (!IsValid(OwnerCharacter_Injected)
		|| !IsValid(HealthComp_Injected)
		|| !HealthComp_Injected->IsAlive()
		|| !IsValid(StateComp_Injected)
		|| StateComp_Injected->GetCurrentExecutionState() != EExecutionState::Idle
		|| !IsValid(ActionComp_Injected)
		|| ActionComp_Injected->IsActive()
		|| !IsValid(ActionOrchestratorComp_Injected))
	{
		return false;
	}

	FActionDataKey actionDataKey;
	actionDataKey.ActionType = EActionType::Execution;
	actionDataKey.ActionIndex = CActionIndexConstants::FirstActionIndex;

	FActionData actionData;
	return ActionComp_Injected->ResolveActionData(actionDataKey, actionData)
		&& actionData.IsValidMinimal()
		&& IsValid(ActionComp_Injected->ResolveActionExecutor(actionData));
}

bool UCExecutionCollaborationComponent::CanStartTargetExecution()
{
	if (!IsValid(OwnerCharacter_Injected)
		|| !IsValid(HealthComp_Injected)
		|| !HealthComp_Injected->IsAlive()
		|| !IsValid(StateComp_Injected)
		|| StateComp_Injected->GetCurrentExecutionState() != EExecutionState::Idle
		|| !IsValid(BalanceComp_Injected)
		|| !BalanceComp_Injected->IsExecutionOpportunityAvailable()
		|| !IsValid(ReactionComp_Injected)
		|| ReactionComp_Injected->IsActive()
		|| !IsValid(ReactionOrchestratorComp_Injected))
	{
		return false;
	}

	FReactionDataKey reactionDataKey;
	reactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	reactionDataKey.ReactionType = EReactionType::Execution;
	reactionDataKey.ReactionIndex = INDEX_NONE;

	FReactionData reactionData;
	return ReactionComp_Injected->ResolveReactionData(reactionDataKey, reactionData)
		&& reactionData.IsValidMinimal()
		&& IsValid(ReactionComp_Injected->ResolveReactionExecutor(reactionData));
}

bool UCExecutionCollaborationComponent::IsSourceTargetSnapshotCurrent() const
{
	if (!bIsSourceRole || !IsValid(CombatTargetComp_Injected)) return false;
	const FCombatTargetSnapshot currentSnapshot = CombatTargetComp_Injected->GetCombatTargetSnapshot();
	return currentSnapshot.TargetActor == ActiveContext.TargetActor
		&& currentSnapshot.Revision == ActiveContext.SourceTargetRevision;
}

bool UCExecutionCollaborationComponent::IsTargetExecutionOpportunityCurrent() const
{
	if (!bIsSourceRole)
	{
		return IsValid(BalanceComp_Injected)
			&& BalanceComp_Injected->IsExecutionOpportunityReservationCurrent(ActiveContext.OpportunityReservation);
	}

	UCExecutionCollaborationComponent* targetCollaborationComp = FindPartnerCollaborationComponent();
	return IsValid(targetCollaborationComp)
		&& targetCollaborationComp->IsActiveSession(ActiveContext.SessionId)
		&& targetCollaborationComp->IsTargetExecutionOpportunityCurrent();
}

UCExecutionCollaborationComponent* UCExecutionCollaborationComponent::FindPartnerCollaborationComponent() const
{
	if (!ActiveContext.IsValidMinimal()) return nullptr;
	const AActor* partnerActor = bIsSourceRole ? ActiveContext.TargetActor : ActiveContext.SessionId.SourceActor;
	return IsValid(partnerActor) ? partnerActor->FindComponentByClass<UCExecutionCollaborationComponent>() : nullptr;
}

uint32 UCExecutionCollaborationComponent::AllocateSessionSerial()
{
	if (NextSessionSerial == 0)
	{
		NextSessionSerial = 1;
	}

	return NextSessionSerial++;
}

void UCExecutionCollaborationComponent::ResetActiveExecutionSession()
{
	ActiveContext = FExecutionCollaborationContext();
	CollaborationState = EExecutionCollaborationState::None;
	bIsSourceRole = false;
	bSourceActionTerminal = false;
	bTargetReactionTerminal = false;
}
