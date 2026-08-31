#include "Component/CExecutionCollaborationComponent.h"

#include "Core/Debug/FExecutionCollaborationDebug.h"
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

bool UCExecutionCollaborationComponent::RequestCombatExecution()
{
	if (HasActiveExecutionSession() || !CanStartSourceExecution()) return false;
	if (!IsValid(CombatTargetComp_Injected) || !IsValid(ActionOrchestratorComp_Injected)) return false;

	const FCombatTargetSnapshot targetSnapshot = CombatTargetComp_Injected->GetCombatTargetSnapshot();
	if (!IsValid(targetSnapshot.TargetActor) || targetSnapshot.Revision <= 0) return false;

	if (!IsSourceExecutionStartGeometryValid(targetSnapshot)) return false;

	UCExecutionCollaborationComponent* targetCollaborationComp = targetSnapshot.TargetActor->FindComponentByClass<UCExecutionCollaborationComponent>();
	if (!IsValid(targetCollaborationComp)) return false;

	FExecutionSessionId sessionId;
	sessionId.SourceActor = OwnerCharacter_Injected;
	sessionId.Serial = AllocateSessionSerial();

	FExecutionCollaborationContext context;
	if (!targetCollaborationComp->AcceptExecutionReservation(sessionId, targetSnapshot, context)) return false;

	ActiveContext = context;
	CollaborationState = EExecutionCollaborationState::Reserved;
	bIsSourceRole = true;
	bSourceActionTerminal = false;
	bTargetReactionTerminal = false;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("ReservationAccepted"));

	if (!CanResolveSourceExecutionAction(ActiveContext.OutcomePolicy))
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::SourceActionRejected, true);
		return false;
	}

	if (!AlignTargetExecutionFacing(ActiveContext.TargetSnapshot))
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::InvalidParticipant, true);
		return false;
	}

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

	if (!ActivateExecutionPair())
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::InvalidParticipant, true);
		return false;
	}

	return true;
}

// Source Commit

bool UCExecutionCollaborationComponent::HandleSourceExecutionCommit(const uint32 InActionRequestSerial, const float InStandardExecutionDamage)
{
	if (!bIsSourceRole || CollaborationState != EExecutionCollaborationState::Active) return false;
	if (InActionRequestSerial == 0 || InActionRequestSerial != ActiveContext.SessionId.Serial) return false;
	if (ActiveContext.OutcomePolicy == EExecutionOutcomePolicy::Standard && InStandardExecutionDamage <= KINDA_SMALL_NUMBER) return false;

	if (!IsTargetSnapshotCurrent())
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
	outcomePacket.StandardExecutionDamage = InStandardExecutionDamage;

	if (!targetSignalComp->RequestExecutionOutcomeTarget(outcomePacket))
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::BalanceOpportunityInvalidated, true);
		return false;
	}

	if (IsActiveSession(outcomePacket.CollaborationContext.SessionId))
	{
		CollaborationState = EExecutionCollaborationState::Committed;
		FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("PartnerCommitted"));
	}

	return true;
}

// Target Outcome

bool UCExecutionCollaborationComponent::CommitExecutionOutcome(const FExecutionOutcomePacket& InPacket)
{
	const FExecutionCollaborationContext& context = InPacket.CollaborationContext;

	if (!IsActiveSession(context.SessionId) || bIsSourceRole || !context.IsValidMinimal()) return false;
	if (CollaborationState != EExecutionCollaborationState::Active) return false;

	if (!ActiveContext.OpportunityReservation.Matches(context.OpportunityReservation)
		|| ActiveContext.TargetSnapshot.TargetActor != context.TargetSnapshot.TargetActor
		|| ActiveContext.TargetSnapshot.Revision != context.TargetSnapshot.Revision
		|| ActiveContext.OutcomePolicy != context.OutcomePolicy)
	{
		return false;
	}

	if (!IsValid(BalanceComp_Injected) || !BalanceComp_Injected->CommitExecutionOpportunityReservation(context.OpportunityReservation))
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::BalanceOpportunityInvalidated, true);
		return false;
	}

	CollaborationState = EExecutionCollaborationState::Committed;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("OutcomeCommitted"));

	if (context.OutcomePolicy == EExecutionOutcomePolicy::Lethal)
	{
		OnExecutionLethalDeathEntryExpected.Broadcast(context.SessionId);
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
	return ActiveContext.IsValidMinimal() && CollaborationState != EExecutionCollaborationState::None;
}

FExecutionCollaborationRuntimeSnapshot UCExecutionCollaborationComponent::GetExecutionCollaborationRuntimeSnapshot() const
{
	FExecutionCollaborationRuntimeSnapshot snapshot;
	snapshot.bHasActiveSession = HasActiveExecutionSession();
	snapshot.bIsSourceRole = bIsSourceRole;
	snapshot.CollaborationState = CollaborationState;
	snapshot.CollaborationContext = ActiveContext;
	snapshot.bSourceActionTerminal = bSourceActionTerminal;
	snapshot.bTargetReactionTerminal = bTargetReactionTerminal;
	return snapshot;
}

bool UCExecutionCollaborationComponent::BuildSourceExecutionStartGeometrySnapshot(FExecutionStartGeometrySnapshot& OutSnapshot) const
{
	OutSnapshot = FExecutionStartGeometrySnapshot();
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CombatTargetComp_Injected) || !StartGeometrySettings.IsValid()) return false;

	const FCombatTargetSnapshot targetSnapshot = CombatTargetComp_Injected->GetCombatTargetSnapshot();
	ACharacter* targetCharacter = Cast<ACharacter>(targetSnapshot.TargetActor);
	if (!IsValid(targetCharacter)) return false;

	OutSnapshot.bHasTarget = true;
	OutSnapshot.TargetActor = targetCharacter;
	OutSnapshot.MaxDistance = StartGeometrySettings.MaxStartDistance;
	OutSnapshot.MaxFacingAngleDegrees = StartGeometrySettings.MaxSourceFacingAngleDegrees;

	FVector sourceForward2D = OwnerCharacter_Injected->GetActorForwardVector();
	sourceForward2D.Z = 0.f;
	if (!sourceForward2D.Normalize()) return false;

	FVector sourceToTarget2D = targetCharacter->GetActorLocation() - OwnerCharacter_Injected->GetActorLocation();
	sourceToTarget2D.Z = 0.f;
	OutSnapshot.CurrentDistance = sourceToTarget2D.Size();
	OutSnapshot.bIsWithinDistance = OutSnapshot.CurrentDistance > KINDA_SMALL_NUMBER
		&& OutSnapshot.CurrentDistance <= OutSnapshot.MaxDistance;
	if (OutSnapshot.CurrentDistance <= KINDA_SMALL_NUMBER) return false;

	sourceToTarget2D /= OutSnapshot.CurrentDistance;
	const float dot = FMath::Clamp(FVector::DotProduct(sourceForward2D, sourceToTarget2D), -1.f, 1.f);
	OutSnapshot.CurrentFacingAngleDegrees = FMath::RadiansToDegrees(FMath::Acos(dot));
	OutSnapshot.bIsWithinFacingAngle = OutSnapshot.CurrentFacingAngleDegrees <= OutSnapshot.MaxFacingAngleDegrees;
	OutSnapshot.bIsValid = OutSnapshot.bIsWithinDistance && OutSnapshot.bIsWithinFacingAngle;
	return true;
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
		FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("SourceTerminal"));

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
			FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("SourceTerminalInterrupted"));
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
	if (InEvent.Context.ExecutionSessionId != ActiveContext.SessionId || InEvent.Context.ReactionDataKey.ReactionType != GetPrimaryReactionType())
	{
		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed)
	{
		if (CollaborationState != EExecutionCollaborationState::Committed)
		{
			CancelActiveExecutionSession(EExecutionCollaborationCancelReason::TargetReactionInterrupted, true);
			return;
		}

		bTargetReactionTerminal = true;
		FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("TargetTerminal"));

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
			FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("TargetTerminalInterrupted"));
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
		if (!bIsSourceRole
			&& CollaborationState == EExecutionCollaborationState::Committed
			&& ActiveContext.OutcomePolicy == EExecutionOutcomePolicy::Lethal)
		{
			return;
		}

		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::ParticipantDeath, true);
	}
}

void UCExecutionCollaborationComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	if (bIsSourceRole
		&& CollaborationState != EExecutionCollaborationState::Committed
		&& HasActiveExecutionSession()
		&& !IsTargetSnapshotCurrent())
	{
		CancelActiveExecutionSession(EExecutionCollaborationCancelReason::TargetChanged, true);
	}
}

// Partner Coordination

bool UCExecutionCollaborationComponent::AcceptExecutionReservation(const FExecutionSessionId& InSessionId, const FCombatTargetSnapshot& InTargetSnapshot, FExecutionCollaborationContext& OutContext)
{
	OutContext = FExecutionCollaborationContext();

	if (HasActiveExecutionSession()
		|| !InSessionId.IsValidMinimal()
		|| !IsValid(InTargetSnapshot.TargetActor)
		|| InTargetSnapshot.TargetActor != OwnerCharacter_Injected
		|| InTargetSnapshot.Revision <= 0)
	{
		return false;
	}

	if (!CanStartTargetExecution()) return false;

	const EExecutionOutcomePolicy outcomePolicy = ResolveTargetExecutionOutcomePolicy();
	if (outcomePolicy == EExecutionOutcomePolicy::None || outcomePolicy == EExecutionOutcomePolicy::Max) return false;

	if (!CanResolveTargetExecutionReaction(outcomePolicy)) return false;

	FExecutionOpportunityReservation reservation;
	if (!BalanceComp_Injected->TryReserveExecutionOpportunity(InSessionId, reservation)) return false;

	FExecutionCollaborationContext context;
	context.SessionId = InSessionId;
	context.TargetSnapshot = InTargetSnapshot;
	context.OpportunityReservation = reservation;
	context.OutcomePolicy = outcomePolicy;

	ActiveContext = context;
	CollaborationState = EExecutionCollaborationState::Reserved;
	bIsSourceRole = false;
	bSourceActionTerminal = false;
	bTargetReactionTerminal = false;

	OutContext = context;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("ReservationAccepted"));
	return true;
}

void UCExecutionCollaborationComponent::ReceivePartnerSourceActionTerminal(const FExecutionSessionId& InSessionId)
{
	if (!IsActiveSession(InSessionId)) return;

	bSourceActionTerminal = true;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("PartnerSourceTerminal"));
	TryCompleteActiveExecutionSession();
}

void UCExecutionCollaborationComponent::ReceivePartnerTargetReactionTerminal(const FExecutionSessionId& InSessionId)
{
	if (!IsActiveSession(InSessionId)) return;

	bTargetReactionTerminal = true;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("PartnerTargetTerminal"));
	TryCompleteActiveExecutionSession();
}

void UCExecutionCollaborationComponent::ReceivePartnerCommit(const FExecutionSessionId& InSessionId)
{
	if (!IsActiveSession(InSessionId)) return;

	CollaborationState = EExecutionCollaborationState::Committed;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("PartnerCommitted"));
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

	return result.IsAccepted();
}

bool UCExecutionCollaborationComponent::StartSourceExecutionAction()
{
	if (!IsValid(ActionOrchestratorComp_Injected)) return false;

	FExecutionActionRequest request;
	request.CollaborationContext = ActiveContext;

	const FActionRequestResult result = ActionOrchestratorComp_Injected->RequestExecutionAction(request);

	return result.IsAccepted();
}

bool UCExecutionCollaborationComponent::ActivateExecutionPair()
{
	if (!bIsSourceRole || CollaborationState != EExecutionCollaborationState::Reserved) return false;

	UCExecutionCollaborationComponent* targetCollaborationComp = FindPartnerCollaborationComponent();

	if (!IsValid(targetCollaborationComp)
		|| !targetCollaborationComp->IsActiveSession(ActiveContext.SessionId)
		|| targetCollaborationComp->bIsSourceRole
		|| targetCollaborationComp->CollaborationState != EExecutionCollaborationState::Reserved)
	{
		return false;
	}

	if (!IsValid(targetCollaborationComp->BalanceComp_Injected)
		|| !targetCollaborationComp->BalanceComp_Injected->ActivateExecutionOpportunityReservation(ActiveContext.OpportunityReservation))
	{
		return false;
	}

	CollaborationState = EExecutionCollaborationState::Active;
	targetCollaborationComp->CollaborationState = EExecutionCollaborationState::Active;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("PairActivated"));
	FExecutionCollaborationDebug::RecordLifecycleEvent(targetCollaborationComp, TEXT("PairActivated"));
	return true;
}

void UCExecutionCollaborationComponent::CancelActiveExecutionSession(const EExecutionCollaborationCancelReason InReason, const bool bNotifyPartner)
{
	if (!HasActiveExecutionSession()) return;

	const FExecutionCollaborationContext context = ActiveContext;
	const FExecutionSessionId sessionId = context.SessionId;
	const bool bWasSourceRole = bIsSourceRole;
	const EReactionType primaryReactionType = GetPrimaryReactionType();
	UCExecutionCollaborationComponent* partnerCollaborationComp = bNotifyPartner ? FindPartnerCollaborationComponent() : nullptr;
	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("Cancelled"), UEnum::GetValueAsString(InReason));

	ResetActiveExecutionSession();

	if (!bWasSourceRole && IsValid(BalanceComp_Injected))
	{
		BalanceComp_Injected->ReleaseExecutionOpportunityReservation(context.OpportunityReservation);
	}

	CancelLocalExecutionParticipant(bWasSourceRole, primaryReactionType);

	if (IsValid(partnerCollaborationComp))
	{
		partnerCollaborationComp->ReceivePartnerCancellation(sessionId, InReason);
	}
}

void UCExecutionCollaborationComponent::CompleteActiveExecutionSession()
{
	if (!HasActiveExecutionSession()) return;

	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("Completed"));
	ResetActiveExecutionSession();
}

void UCExecutionCollaborationComponent::TryCompleteActiveExecutionSession()
{
	if (CollaborationState != EExecutionCollaborationState::Committed) return;
	if (!bSourceActionTerminal || !bTargetReactionTerminal) return;
	if (!bIsSourceRole && ActiveContext.OutcomePolicy == EExecutionOutcomePolicy::Standard)
	{
		if (!IsValid(BalanceComp_Injected) || !BalanceComp_Injected->EnterExecutionDown(ActiveContext.OpportunityReservation.BalanceLifecycleSerial))
		{
			CancelActiveExecutionSession(EExecutionCollaborationCancelReason::BalanceOpportunityInvalidated, true);
			return;
		}
	}

	CompleteActiveExecutionSession();
}

void UCExecutionCollaborationComponent::CancelLocalExecutionParticipant(const bool bWasSourceRole, const EReactionType InPrimaryReactionType)
{
	if (bWasSourceRole)
	{
		if (IsValid(ActionComp_Injected) && ActionComp_Injected->IsActiveActionType(EActionType::Execution))
		{
			ActionComp_Injected->CancelActiveActionForSystem();
		}
		return;
	}

	if (IsValid(ReactionComp_Injected) && ReactionComp_Injected->IsActiveReactionType(InPrimaryReactionType))
	{
		ReactionComp_Injected->CancelActiveReactionForSystem();
	}
}

// Session Validation

bool UCExecutionCollaborationComponent::IsActiveSession(const FExecutionSessionId& InSessionId) const
{
	return HasActiveExecutionSession() && ActiveContext.SessionId == InSessionId;
}

bool UCExecutionCollaborationComponent::IsTargetSnapshotCurrent() const
{
	if (!bIsSourceRole || !IsValid(CombatTargetComp_Injected)) return false;
	const FCombatTargetSnapshot currentSnapshot = CombatTargetComp_Injected->GetCombatTargetSnapshot();
	return currentSnapshot.TargetActor == ActiveContext.TargetSnapshot.TargetActor
		&& currentSnapshot.Revision == ActiveContext.TargetSnapshot.Revision;
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

// Execution Startup Validation

bool UCExecutionCollaborationComponent::CanStartSourceExecution() const
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

	return true;
}

bool UCExecutionCollaborationComponent::CanResolveSourceExecutionAction(const EExecutionOutcomePolicy InOutcomePolicy) const
{
	if ((InOutcomePolicy != EExecutionOutcomePolicy::Standard && InOutcomePolicy != EExecutionOutcomePolicy::Lethal)
		|| !IsValid(ActionComp_Injected))
	{
		return false;
	}

	FActionDataKey actionDataKey;
	actionDataKey.ActionType = EActionType::Execution;
	actionDataKey.ActionIndex = GetExecutionActionIndex(InOutcomePolicy);

	FActionData actionData;
	if (!ActionComp_Injected->ResolveActionData(actionDataKey, actionData)
		|| !actionData.IsValidMinimal()
		|| !IsValid(ActionComp_Injected->ResolveActionExecutor(actionData)))
	{
		return false;
	}

	return InOutcomePolicy != EExecutionOutcomePolicy::Standard
		|| actionData.StandardExecutionDamage > KINDA_SMALL_NUMBER;
}

bool UCExecutionCollaborationComponent::CanStartTargetExecution() const
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

	return true;
}

bool UCExecutionCollaborationComponent::CanResolveTargetExecutionReaction(const EExecutionOutcomePolicy InOutcomePolicy) const
{
	if ((InOutcomePolicy != EExecutionOutcomePolicy::Standard && InOutcomePolicy != EExecutionOutcomePolicy::Lethal) || !IsValid(ReactionComp_Injected))
	{
		return false;
	}

	FReactionDataKey reactionDataKey;
	reactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	reactionDataKey.ReactionType = InOutcomePolicy == EExecutionOutcomePolicy::Lethal ? EReactionType::ExecutionLethal : EReactionType::ExecutionStandard;
	reactionDataKey.ReactionIndex = INDEX_NONE;

	FReactionData reactionData;
	return ReactionComp_Injected->ResolveReactionData(reactionDataKey, reactionData)
		&& reactionData.IsValidMinimal()
		&& IsValid(ReactionComp_Injected->ResolveReactionExecutor(reactionData));
}

bool UCExecutionCollaborationComponent::IsSourceExecutionStartGeometryValid(const FCombatTargetSnapshot& InTargetSnapshot) const
{
	if (!StartGeometrySettings.IsValid()
		|| !IsValid(OwnerCharacter_Injected)
		|| !IsValid(InTargetSnapshot.TargetActor))
	{
		return false;
	}

	ACharacter* targetCharacter = Cast<ACharacter>(InTargetSnapshot.TargetActor);
	if (!IsValid(targetCharacter)) return false;

	FVector sourceForward2D = OwnerCharacter_Injected->GetActorForwardVector();
	sourceForward2D.Z = 0.f;
	
	if (!sourceForward2D.Normalize()) return false;

	const FVector sourceLocation = OwnerCharacter_Injected->GetActorLocation();
	const FVector targetLocation = targetCharacter->GetActorLocation();
	FVector sourceToTarget2D = targetLocation - sourceLocation;
	sourceToTarget2D.Z = 0.f;

	const float currentDistance = sourceToTarget2D.Size();
	if (currentDistance <= KINDA_SMALL_NUMBER || currentDistance > StartGeometrySettings.MaxStartDistance) return false;

	sourceToTarget2D /= currentDistance;
	const float dot = FMath::Clamp(FVector::DotProduct(sourceForward2D, sourceToTarget2D), -1.f, 1.f);
	const float angleDegrees = FMath::RadiansToDegrees(FMath::Acos(dot));

	return angleDegrees <= StartGeometrySettings.MaxSourceFacingAngleDegrees;
}

bool UCExecutionCollaborationComponent::AlignTargetExecutionFacing(const FCombatTargetSnapshot& InTargetSnapshot) const
{
	if (!IsValid(OwnerCharacter_Injected)) return false;

	ACharacter* targetCharacter = Cast<ACharacter>(InTargetSnapshot.TargetActor);
	if (!IsValid(targetCharacter)) return false;

	FVector targetToSource2D = OwnerCharacter_Injected->GetActorLocation() - targetCharacter->GetActorLocation();
	targetToSource2D.Z = 0.f;
	if (!targetToSource2D.Normalize()) return false;

	FRotator targetFacingRotation = targetToSource2D.Rotation();
	targetFacingRotation.Pitch = 0.f;
	targetFacingRotation.Roll = 0.f;
	targetCharacter->SetActorRotation(targetFacingRotation);
	return true;
}

// Target Outcome Resolution

EExecutionOutcomePolicy UCExecutionCollaborationComponent::ResolveTargetExecutionOutcomePolicy() const
{
	return CanResolveLethalExecutionOutcome()
		? EExecutionOutcomePolicy::Lethal
		: EExecutionOutcomePolicy::Standard;
}

bool UCExecutionCollaborationComponent::CanResolveLethalExecutionOutcome() const
{
	if (LethalCondition != EExecutionLethalCondition::HealthRatio
		|| !IsValid(HealthComp_Injected)
		|| !HealthComp_Injected->CanKill())
	{
		return false;
	}

	const float maxHealth = HealthComp_Injected->GetMaxHP();
	if (maxHealth <= KINDA_SMALL_NUMBER) return false;

	const float currentHealthRatio = HealthComp_Injected->GetCurrentHP() / maxHealth;
	return currentHealthRatio <= LethalHealthRatio;
}

// Execution Policy Resolution

EReactionType UCExecutionCollaborationComponent::GetPrimaryReactionType() const
{
	return ActiveContext.OutcomePolicy == EExecutionOutcomePolicy::Lethal
		? EReactionType::ExecutionLethal
		: EReactionType::ExecutionStandard;
}

int32 UCExecutionCollaborationComponent::GetExecutionActionIndex(const EExecutionOutcomePolicy InOutcomePolicy) const
{
	return InOutcomePolicy == EExecutionOutcomePolicy::Lethal
		? CExecutionActionIndex::Lethal
		: CExecutionActionIndex::Standard;
}

// Partner Lookup

UCExecutionCollaborationComponent* UCExecutionCollaborationComponent::FindPartnerCollaborationComponent() const
{
	if (!ActiveContext.IsValidMinimal()) return nullptr;

	const AActor* partnerActor = bIsSourceRole ? ActiveContext.TargetSnapshot.TargetActor : ActiveContext.SessionId.SourceActor;

	return IsValid(partnerActor) ? partnerActor->FindComponentByClass<UCExecutionCollaborationComponent>() : nullptr;
}

// Session Runtime

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
