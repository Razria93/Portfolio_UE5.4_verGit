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

EExternalCombatInputPolicy UCExecutionCollaborationComponent::GetExternalCombatInputPolicy() const
{
	// Both collaboration participants are protected while their shared session
	// owns the cinematic action/reaction sequence.
	if (HasActiveExecutionSession())
	{
		return EExternalCombatInputPolicy::RejectAll;
	}

	if (!IsValid(BalanceComp_Injected))
	{
		return EExternalCombatInputPolicy::Normal;
	}

	switch (BalanceComp_Injected->GetBalanceLifecycleState())
	{
	case EBalanceLifecycleState::ExecutionDownActive:
	case EBalanceLifecycleState::ExecutionRecoveryPending:
	case EBalanceLifecycleState::ExecutionRecoveryActive:
		return EExternalCombatInputPolicy::DamageOnly;

	default:
		return EExternalCombatInputPolicy::Normal;
	}
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
	FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceRequestReceived"));
	if (HasActiveExecutionSession())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceRequestRejected"), TEXT("SourceSessionAlreadyActive"));
		return false;
	}

	if (!CanStartSourceExecution()) return false;
	if (!IsValid(CombatTargetComp_Injected) || !IsValid(ActionOrchestratorComp_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceRequestRejected"), TEXT("MissingSourceTargetOrActionOrchestrator"));
		return false;
	}

	const FCombatTargetSnapshot targetSnapshot = CombatTargetComp_Injected->GetCombatTargetSnapshot();
	if (!IsValid(targetSnapshot.TargetActor) || targetSnapshot.Revision <= 0)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceRequestRejected"), TEXT("InvalidCombatTargetSnapshot"));
		return false;
	}

	if (!IsSourceExecutionStartGeometryValid(targetSnapshot)) return false;

	UCExecutionCollaborationComponent* targetCollaborationComp = targetSnapshot.TargetActor->FindComponentByClass<UCExecutionCollaborationComponent>();
	if (!IsValid(targetCollaborationComp))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceRequestRejected"), TEXT("MissingTargetCollaborationComponent"));
		return false;
	}

	FExecutionSessionId sessionId;
	sessionId.SourceActor = OwnerCharacter_Injected;
	sessionId.Serial = AllocateSessionSerial();

	const float standardExecutionDamage = ResolveStandardExecutionDamageForReservation();
	FExecutionCollaborationContext context;
	if (!targetCollaborationComp->AcceptExecutionReservation(sessionId, targetSnapshot, standardExecutionDamage, context))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceRequestRejected"), TEXT("TargetReservationRejected"));
		return false;
	}

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
	FExecutionCollaborationDebug::RecordStartTrace(
		OwnerCharacter_Injected,
		TEXT("SourceCommitHandlerEntered"),
		FString::Printf(
			TEXT("Role=%s | State=%s | ActionSerial=%u | SessionSerial=%u | Damage=%.1f"),
			bIsSourceRole ? TEXT("Source") : TEXT("Target"),
			*UEnum::GetValueAsString(CollaborationState),
			InActionRequestSerial,
			ActiveContext.SessionId.Serial,
			InStandardExecutionDamage));

	if (!bIsSourceRole)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceCommitRejected"), TEXT("NotSourceRole"));
		return false;
	}
	if (CollaborationState != EExecutionCollaborationState::Active)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceCommitRejected"), FString::Printf(TEXT("CollaborationState=%s"), *UEnum::GetValueAsString(CollaborationState)));
		return false;
	}
	if (InActionRequestSerial == 0 || InActionRequestSerial != ActiveContext.SessionId.Serial)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceCommitRejected"), FString::Printf(TEXT("ActionSerial=%u | SessionSerial=%u"), InActionRequestSerial, ActiveContext.SessionId.Serial));
		return false;
	}
	if (ActiveContext.OutcomePolicy == EExecutionOutcomePolicy::Standard && InStandardExecutionDamage <= KINDA_SMALL_NUMBER)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceCommitRejected"), TEXT("StandardDamageIsZero"));
		return false;
	}

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
	FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceCommitDispatchTargetOutcome"));

	if (!targetSignalComp->RequestExecutionOutcomeTarget(outcomePacket))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceCommitRejected"), TEXT("TargetOutcomeRejected"));
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
	FExecutionCollaborationDebug::RecordStartTrace(
		OwnerCharacter_Injected,
		TEXT("TargetOutcomeCommitHandlerEntered"),
		FString::Printf(
			TEXT("Role=%s | State=%s | PacketSession=%u | ActiveSession=%u | Outcome=%s"),
			bIsSourceRole ? TEXT("Source") : TEXT("Target"),
			*UEnum::GetValueAsString(CollaborationState),
			context.SessionId.Serial,
			ActiveContext.SessionId.Serial,
			*UEnum::GetValueAsString(context.OutcomePolicy)));

	if (!IsActiveSession(context.SessionId) || bIsSourceRole || !context.IsValidMinimal())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetOutcomeCommitRejected"), TEXT("InvalidSessionRoleOrContext"));
		return false;
	}
	if (CollaborationState != EExecutionCollaborationState::Active)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetOutcomeCommitRejected"), FString::Printf(TEXT("CollaborationState=%s"), *UEnum::GetValueAsString(CollaborationState)));
		return false;
	}

	if (!ActiveContext.OpportunityReservation.Matches(context.OpportunityReservation)
		|| ActiveContext.TargetSnapshot.TargetActor != context.TargetSnapshot.TargetActor
		|| ActiveContext.TargetSnapshot.Revision != context.TargetSnapshot.Revision
		|| ActiveContext.OutcomePolicy != context.OutcomePolicy)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetOutcomeCommitRejected"), TEXT("ContextMismatch"));
		return false;
	}

	if (!IsValid(BalanceComp_Injected) || !BalanceComp_Injected->CommitExecutionOpportunityReservation(context.OpportunityReservation))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetOutcomeCommitRejected"), TEXT("BalanceCommitRejected"));
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

	FExecutionCollaborationDebug::RecordStartTrace(
		OwnerCharacter_Injected,
		TEXT("TargetReactionLifecycleReceived"),
		FString::Printf(
			TEXT("Event=%s | Finish=%s | CollaborationState=%s"),
			*UEnum::GetValueAsString(InEvent.EventType),
			*UEnum::GetValueAsString(InEvent.FinishReason),
			*UEnum::GetValueAsString(CollaborationState)));

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

bool UCExecutionCollaborationComponent::AcceptExecutionReservation(const FExecutionSessionId& InSessionId, const FCombatTargetSnapshot& InTargetSnapshot, const float InStandardExecutionDamage, FExecutionCollaborationContext& OutContext)
{
	OutContext = FExecutionCollaborationContext();

	if (HasActiveExecutionSession()
		|| !InSessionId.IsValidMinimal()
		|| !IsValid(InTargetSnapshot.TargetActor)
		|| InTargetSnapshot.TargetActor != OwnerCharacter_Injected
		|| InTargetSnapshot.Revision <= 0)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetReservationRejected"), TEXT("InvalidSessionOrTargetSnapshot"));
		return false;
	}

	if (!CanStartTargetExecution()) return false;

	const EExecutionOutcomePolicy outcomePolicy = ResolveTargetExecutionOutcomePolicy();
	if (outcomePolicy == EExecutionOutcomePolicy::None || outcomePolicy == EExecutionOutcomePolicy::Max)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetReservationRejected"), TEXT("OutcomePolicyUnavailable"));
		return false;
	}

	float appliedDamage = 0.f;
	if (!IsValid(CombatSignalTargetComp_Injected)
		|| !CombatSignalTargetComp_Injected->TryResolveExecutionAppliedDamage(outcomePolicy, InStandardExecutionDamage, appliedDamage))
	{
		const float currentHealth = IsValid(HealthComp_Injected) ? HealthComp_Injected->GetCurrentHP() : 0.f;
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetReservationRejected"), FString::Printf(TEXT("OutcomeNotApplicable | Outcome=%s | CurrentHP=%.1f | StandardDamage=%.1f"), *UEnum::GetValueAsString(outcomePolicy), currentHealth, InStandardExecutionDamage));
		return false;
	}

	if (!CanResolveTargetExecutionReaction(outcomePolicy)) return false;

	FExecutionOpportunityReservation reservation;
	if (!BalanceComp_Injected->TryReserveExecutionOpportunity(InSessionId, reservation))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetReservationRejected"), TEXT("BalanceReservationRejected"));
		return false;
	}

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
	if (!IsValid(targetCollaborationComp) || !IsValid(targetCollaborationComp->ReactionOrchestratorComp_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetReactionStartRejected"), TEXT("MissingTargetReactionOrchestrator"));
		return false;
	}

	FExecutionReactionRequest request;
	request.CollaborationContext = ActiveContext;

	const FReactionRequestResult result = targetCollaborationComp->ReactionOrchestratorComp_Injected->RequestExecutionReaction(request);
	if (!result.IsAccepted())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetReactionStartRejected"), UEnum::GetValueAsString(result.RejectReason));
	}

	return result.IsAccepted();
}

bool UCExecutionCollaborationComponent::StartSourceExecutionAction()
{
	if (!IsValid(ActionOrchestratorComp_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceActionStartRejected"), TEXT("MissingSourceActionOrchestrator"));
		return false;
	}

	FExecutionActionRequest request;
	request.CollaborationContext = ActiveContext;

	const FActionRequestResult result = ActionOrchestratorComp_Injected->RequestExecutionAction(request);
	if (!result.IsAccepted())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceActionStartRejected"), UEnum::GetValueAsString(result.RejectReason));
	}

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

	if (!ApplyExecutionParticipantMovementIgnore(targetCollaborationComp))
	{
		CollaborationState = EExecutionCollaborationState::Reserved;
		targetCollaborationComp->CollaborationState = EExecutionCollaborationState::Reserved;
		return false;
	}

	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("PairActivated"));
	FExecutionCollaborationDebug::RecordLifecycleEvent(targetCollaborationComp, TEXT("PairActivated"));
	return true;
}

// Participant Movement Collision Policy

bool UCExecutionCollaborationComponent::ApplyExecutionParticipantMovementIgnore(UCExecutionCollaborationComponent* const InPartnerComponent)
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(InPartnerComponent) || !IsValid(InPartnerComponent->OwnerCharacter_Injected)) return false;

	ACharacter* const partnerCharacter = InPartnerComponent->OwnerCharacter_Injected;
	if (partnerCharacter == OwnerCharacter_Injected) return false;

	if (MovementIgnoredExecutionPartner.IsValid() || InPartnerComponent->MovementIgnoredExecutionPartner.IsValid())
	{
		return MovementIgnoredExecutionPartner.Get() == partnerCharacter
			&& InPartnerComponent->MovementIgnoredExecutionPartner.Get() == OwnerCharacter_Injected;
	}

	OwnerCharacter_Injected->MoveIgnoreActorAdd(partnerCharacter);
	partnerCharacter->MoveIgnoreActorAdd(OwnerCharacter_Injected);

	MovementIgnoredExecutionPartner = partnerCharacter;
	InPartnerComponent->MovementIgnoredExecutionPartner = OwnerCharacter_Injected;

	FExecutionCollaborationDebug::RecordLifecycleEvent(this, TEXT("PairMovementIgnoreApplied"), FString::Printf(TEXT("Partner=%s"), *GetNameSafe(partnerCharacter)));
	return true;
}

void UCExecutionCollaborationComponent::RestoreExecutionParticipantMovementIgnore()
{
	ACharacter* const partnerCharacter = MovementIgnoredExecutionPartner.Get();
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(partnerCharacter))
	{
		MovementIgnoredExecutionPartner.Reset();
		return;
	}

	OwnerCharacter_Injected->MoveIgnoreActorRemove(partnerCharacter);
	MovementIgnoredExecutionPartner.Reset();

	FExecutionCollaborationDebug::RecordLifecycleEvent(
		this,
		TEXT("ParticipantMovementIgnoreRestored"),
		FString::Printf(TEXT("Partner=%s"), *GetNameSafe(partnerCharacter)));
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
		if (!IsValid(BalanceComp_Injected) || !BalanceComp_Injected->EnterExecutionDownLifecycle(ActiveContext.OpportunityReservation.BalanceLifecycleSerial))
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
	if (!IsValid(OwnerCharacter_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(this, TEXT("SourcePreconditionRejected"), TEXT("InvalidOwner"));
		return false;
	}

	if (!IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourcePreconditionRejected"), TEXT("SourceNotAlive"));
		return false;
	}

	if (!IsValid(StateComp_Injected) || StateComp_Injected->GetCurrentExecutionState() != EExecutionState::Idle)
	{
		const FString stateText = IsValid(StateComp_Injected)
			? UEnum::GetValueAsString(StateComp_Injected->GetCurrentExecutionState())
			: TEXT("InvalidStateComponent");
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourcePreconditionRejected"), FString::Printf(TEXT("SourceState=%s"), *stateText));
		return false;
	}

	if (!IsValid(ActionComp_Injected) || ActionComp_Injected->IsActive())
	{
		const FString actionText = IsValid(ActionComp_Injected)
			? UEnum::GetValueAsString(ActionComp_Injected->GetActiveActionType())
			: TEXT("InvalidActionComponent");
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourcePreconditionRejected"), FString::Printf(TEXT("ActiveAction=%s"), *actionText));
		return false;
	}

	if (!IsValid(ActionOrchestratorComp_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourcePreconditionRejected"), TEXT("InvalidActionOrchestrator"));
		return false;
	}

	return true;
}

bool UCExecutionCollaborationComponent::CanResolveSourceExecutionAction(const EExecutionOutcomePolicy InOutcomePolicy) const
{
	if ((InOutcomePolicy != EExecutionOutcomePolicy::Standard && InOutcomePolicy != EExecutionOutcomePolicy::Lethal)
		|| !IsValid(ActionComp_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceDataRejected"), TEXT("InvalidOutcomeOrActionComponent"));
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
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceDataRejected"), FString::Printf(TEXT("Action=%s | Index=%d | MissingDataOrExecutor"), *UEnum::GetValueAsString(actionDataKey.ActionType), actionDataKey.ActionIndex));
		return false;
	}

	if (InOutcomePolicy == EExecutionOutcomePolicy::Standard && actionData.StandardExecutionDamage <= KINDA_SMALL_NUMBER)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("SourceDataRejected"), TEXT("StandardExecutionDamageIsZero"));
		return false;
	}

	return true;
}

float UCExecutionCollaborationComponent::ResolveStandardExecutionDamageForReservation() const
{
	if (!IsValid(ActionComp_Injected)) return 0.f;

	FActionDataKey actionDataKey;
	actionDataKey.ActionType = EActionType::Execution;
	actionDataKey.ActionIndex = GetExecutionActionIndex(EExecutionOutcomePolicy::Standard);

	FActionData actionData;
	return ActionComp_Injected->ResolveActionData(actionDataKey, actionData) && actionData.IsValidMinimal()
		? actionData.StandardExecutionDamage
		: 0.f;
}

bool UCExecutionCollaborationComponent::CanStartTargetExecution() const
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(this, TEXT("TargetPreconditionRejected"), TEXT("InvalidOwner"));
		return false;
	}

	if (!IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetPreconditionRejected"), TEXT("TargetNotAlive"));
		return false;
	}

	if (!IsValid(StateComp_Injected) || StateComp_Injected->GetCurrentExecutionState() != EExecutionState::Idle)
	{
		const FString stateText = IsValid(StateComp_Injected)
			? UEnum::GetValueAsString(StateComp_Injected->GetCurrentExecutionState())
			: TEXT("InvalidStateComponent");
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetPreconditionRejected"), FString::Printf(TEXT("TargetState=%s"), *stateText));
		return false;
	}

	if (!IsValid(BalanceComp_Injected) || !BalanceComp_Injected->IsExecutionOpportunityAvailable())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetPreconditionRejected"), TEXT("CollapseOpportunityUnavailable"));
		return false;
	}

	if (!IsValid(ReactionComp_Injected) || ReactionComp_Injected->IsActive())
	{
		const FString reactionText = IsValid(ReactionComp_Injected)
			? UEnum::GetValueAsString(ReactionComp_Injected->GetActiveReactionType())
			: TEXT("InvalidReactionComponent");
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetPreconditionRejected"), FString::Printf(TEXT("ActiveReaction=%s"), *reactionText));
		return false;
	}

	if (!IsValid(ReactionOrchestratorComp_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetPreconditionRejected"), TEXT("InvalidReactionOrchestrator"));
		return false;
	}

	return true;
}

bool UCExecutionCollaborationComponent::CanResolveTargetExecutionReaction(const EExecutionOutcomePolicy InOutcomePolicy) const
{
	if ((InOutcomePolicy != EExecutionOutcomePolicy::Standard && InOutcomePolicy != EExecutionOutcomePolicy::Lethal) || !IsValid(ReactionComp_Injected))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetDataRejected"), TEXT("InvalidOutcomeOrReactionComponent"));
		return false;
	}

	FReactionDataKey reactionDataKey;
	reactionDataKey.MatchMode = EReactionDataMatchMode::Global;
	reactionDataKey.ReactionType = InOutcomePolicy == EExecutionOutcomePolicy::Lethal ? EReactionType::ExecutionLethal : EReactionType::ExecutionStandard;
	reactionDataKey.ReactionIndex = INDEX_NONE;

	FReactionData reactionData;
	if (!ReactionComp_Injected->ResolveReactionData(reactionDataKey, reactionData)
		|| !reactionData.IsValidMinimal()
		|| !IsValid(ReactionComp_Injected->ResolveReactionExecutor(reactionData)))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("TargetDataRejected"), FString::Printf(TEXT("Reaction=%s | Index=%d | MissingDataOrExecutor"), *UEnum::GetValueAsString(reactionDataKey.ReactionType), reactionDataKey.ReactionIndex));
		return false;
	}

	return true;
}

bool UCExecutionCollaborationComponent::IsSourceExecutionStartGeometryValid(const FCombatTargetSnapshot& InTargetSnapshot) const
{
	if (!StartGeometrySettings.IsValid()
		|| !IsValid(OwnerCharacter_Injected)
		|| !IsValid(InTargetSnapshot.TargetActor))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("GeometryRejected"), TEXT("InvalidSettingsOrParticipant"));
		return false;
	}

	ACharacter* targetCharacter = Cast<ACharacter>(InTargetSnapshot.TargetActor);
	if (!IsValid(targetCharacter))
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("GeometryRejected"), TEXT("TargetIsNotCharacter"));
		return false;
	}

	FVector sourceForward2D = OwnerCharacter_Injected->GetActorForwardVector();
	sourceForward2D.Z = 0.f;

	if (!sourceForward2D.Normalize())
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("GeometryRejected"), TEXT("InvalidSourceForward"));
		return false;
	}

	const FVector sourceLocation = OwnerCharacter_Injected->GetActorLocation();
	const FVector targetLocation = targetCharacter->GetActorLocation();
	FVector sourceToTarget2D = targetLocation - sourceLocation;
	sourceToTarget2D.Z = 0.f;

	const float currentDistance = sourceToTarget2D.Size();
	if (currentDistance <= KINDA_SMALL_NUMBER || currentDistance > StartGeometrySettings.MaxStartDistance)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("GeometryRejected"), FString::Printf(TEXT("Distance=%.1f / %.1f"), currentDistance, StartGeometrySettings.MaxStartDistance));
		return false;
	}

	sourceToTarget2D /= currentDistance;
	const float dot = FMath::Clamp(FVector::DotProduct(sourceForward2D, sourceToTarget2D), -1.f, 1.f);
	const float angleDegrees = FMath::RadiansToDegrees(FMath::Acos(dot));

	if (angleDegrees > StartGeometrySettings.MaxSourceFacingAngleDegrees)
	{
		FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("GeometryRejected"), FString::Printf(TEXT("Angle=%.1f / %.1f"), angleDegrees, StartGeometrySettings.MaxSourceFacingAngleDegrees));
		return false;
	}

	FExecutionCollaborationDebug::RecordStartTrace(OwnerCharacter_Injected, TEXT("GeometryAccepted"), FString::Printf(TEXT("Distance=%.1f / %.1f | Angle=%.1f / %.1f"), currentDistance, StartGeometrySettings.MaxStartDistance, angleDegrees, StartGeometrySettings.MaxSourceFacingAngleDegrees));
	return true;
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
	RestoreExecutionParticipantMovementIgnore();
	ActiveContext = FExecutionCollaborationContext();
	CollaborationState = EExecutionCollaborationState::None;
	bIsSourceRole = false;
	bSourceActionTerminal = false;
	bTargetReactionTerminal = false;
}
