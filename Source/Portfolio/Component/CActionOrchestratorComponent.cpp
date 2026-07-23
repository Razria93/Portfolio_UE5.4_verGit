#include "Component/CActionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CObservableOverlayComponent.h"

#include "Action/CAction.h"
#include "Reaction/CReaction.h"

#include "Core/Debug/FExecutionOrchestratorDebug.h"

#include "Type/CActionOrchestrationTypes.h"

UCActionOrchestratorComponent::UCActionOrchestratorComponent()
{
}

void UCActionOrchestratorComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	MovementComp_Injected = InReferences.MovementComponent;
	WeaponComp_Injected = InReferences.WeaponComponent;
	StateComp_Injected = InReferences.StateComponent;
	HealthComp_Injected = InReferences.HealthComponent;
	ObservableOverlayComp_Injected = InReferences.ObservableOverlayComponent;
	ActionComp_Injected = InReferences.ActionComponent;
	ReactionComp_Injected = InReferences.ReactionComponent;

	ValidateRequiredComponentReferences();
}

bool UCActionOrchestratorComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ MovementComp_Injected, TEXT("UCMovementComponent") },
		{ WeaponComp_Injected, TEXT("UCWeaponComponent") },
		{ StateComp_Injected, TEXT("UCStateComponent") },
		{ HealthComp_Injected, TEXT("UCHealthComponent") },
		{ ActionComp_Injected, TEXT("UCActionComponent") },
		{ ReactionComp_Injected, TEXT("UCReactionComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Request Entry

FActionRequestResult UCActionOrchestratorComponent::RequestMovementAction(const FMovementActionRequest& InIncomingRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Injected) || !IsValid(MovementComp_Injected))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	// Release-style cleanup is allowed before hard-block checks.
	if (InIncomingRequest.IntentType == EMovementActionIntent::StopJump)
	{
		MovementComp_Injected->OnStopJump();
		return BuildActionRequestResult(EActionRequestResultType::Handled);
	}

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	const EExecutionState executionState = IsValid(StateComp_Injected) ? StateComp_Injected->GetCurrentExecutionState() : EExecutionState::Dead;

	if (executionState == EExecutionState::Reaction)
		return BuildActionRequestResult(EActionRequestResultType::Ignored);

	switch (InIncomingRequest.IntentType)
	{
	case EMovementActionIntent::Move:
	{
		MovementComp_Injected->OnMove(InIncomingRequest.Axis2D);
		break;
	}
	case EMovementActionIntent::Walk:
	{
		MovementComp_Injected->OnWalk();
		break;
	}
	case EMovementActionIntent::Run:
	{
		MovementComp_Injected->OnRun();
		break;
	}
	case EMovementActionIntent::Sprint:
	{
		MovementComp_Injected->OnSprint();
		break;
	}
	case EMovementActionIntent::Jump:
	{
		MovementComp_Injected->OnJump();
		break;
	}
	default:
		return BuildActionRequestResult(EActionRequestResultType::Ignored);
	}

	return BuildActionRequestResult(EActionRequestResultType::Handled);
}

FActionRequestResult UCActionOrchestratorComponent::RequestEquipmentAction(const FEquipmentActionRequest& InIncomingRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(WeaponComp_Injected) || !IsValid(ActionComp_Injected))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	FActionCandidate incomingCandidate;

	if (!ResolveEquipmentActionCandidate(InIncomingRequest, incomingCandidate, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	return ProcessActionCandidate(incomingCandidate);
}

FActionRequestResult UCActionOrchestratorComponent::RequestCombatAction(const FCombatActionRequest& InIncomingRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(ActionComp_Injected))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	ApplyCombatActionInputSideEffects(InIncomingRequest);

	FActionCandidate incomingCandidate;

	if (!ResolveCombatActionCandidate(InIncomingRequest, incomingCandidate, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	return ProcessActionCandidate(incomingCandidate);
}

// Deferred Entry

FActionRequestResult UCActionOrchestratorComponent::ConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey)
{
	if (InConsumeKey == EDeferredActionConsumeKey::None || InConsumeKey == EDeferredActionConsumeKey::Max)
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidRequest);

	const int32 foundIndex = DeferredActionCandidates.IndexOfByPredicate(
		[InConsumeKey](const FDeferredActionCandidate& InEntry)
		{
			return InEntry.ConsumeKey == InConsumeKey && InEntry.IsValidMinimal();
		});

	if (foundIndex == INDEX_NONE)
		return BuildActionRequestResult(EActionRequestResultType::Ignored);

	const FActionCandidate candidate = DeferredActionCandidates[foundIndex].Candidate;
	DeferredActionCandidates.RemoveAt(foundIndex);

	return ProcessActionCandidate(candidate);
}

// Deferred Management

void UCActionOrchestratorComponent::ClearAllDeferredActions()
{
	DeferredActionCandidates.Reset();
}

void UCActionOrchestratorComponent::ClearDeferredActions(EDeferredActionConsumeKey InConsumeKey)
{
	if (InConsumeKey == EDeferredActionConsumeKey::None || InConsumeKey == EDeferredActionConsumeKey::Max) return;

	DeferredActionCandidates.RemoveAll(
		[InConsumeKey](const FDeferredActionCandidate& InEntry)
		{
			return InEntry.ConsumeKey == InConsumeKey;
		});
}

void UCActionOrchestratorComponent::ClearDeferredActions(EDeferredActionConsumeKey InConsumeKey, const FActionDataKey& InActionDataKey)
{
	if (InConsumeKey == EDeferredActionConsumeKey::None || InConsumeKey == EDeferredActionConsumeKey::Max) return;
	if (!InActionDataKey.IsValidMinimal()) return;

	FActionCandidate candidate;
	candidate.ActionDataKey = InActionDataKey;

	DeferredActionCandidates.RemoveAll(
		[InConsumeKey, candidate](const FDeferredActionCandidate& InEntry)
		{
			return InEntry.MatchesIdentity(InConsumeKey, candidate);
		});
}

// Request Validation

bool UCActionOrchestratorComponent::CanAcceptActionRequest(EActionRequestRejectReason& OutRejectReason) const
{
	OutRejectReason = EActionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Injected))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidOwner;
		return false;
	}

	if (!IsValid(HealthComp_Injected) || !IsValid(StateComp_Injected))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidComponent;
		return false;
	}

	if (!HealthComp_Injected->IsAlive())
	{
		OutRejectReason = EActionRequestRejectReason::Dead;
		return false;
	}

	const EExecutionState executionState = StateComp_Injected->GetCurrentExecutionState();

	if (executionState == EExecutionState::Dead)
	{
		OutRejectReason = EActionRequestRejectReason::Dead;
		return false;
	}

	return true;
}

// Candidate Resolve

bool UCActionOrchestratorComponent::ResolveEquipmentActionCandidate(const FEquipmentActionRequest& InIncomingRequest, FActionCandidate& OutIncomingCandidate, EActionRequestRejectReason& OutRejectReason) const
{
	OutIncomingCandidate = FActionCandidate();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!IsValid(WeaponComp_Injected))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidComponent;
		return false;
	}

	FActionCandidate incomingCandidate;

	switch (InIncomingRequest.IntentType)
	{
	case EEquipmentActionIntent::Equip:
	{
		incomingCandidate.ActionDataKey.ActionType = EActionType::Equip;
		incomingCandidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	case EEquipmentActionIntent::Unequip:
	{
		incomingCandidate.ActionDataKey.ActionType = EActionType::Unequip;
		incomingCandidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	case EEquipmentActionIntent::Toggle:
	{
		incomingCandidate.ActionDataKey.ActionType = WeaponComp_Injected->CheckCurrentWeaponType(EWeaponType::Unarmed) ? EActionType::Equip : EActionType::Unequip;
		incomingCandidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	default:
		OutRejectReason = EActionRequestRejectReason::InvalidEquipment;
		return false;
	}

	OutIncomingCandidate = incomingCandidate;
	return true;
}

bool UCActionOrchestratorComponent::ResolveCombatActionCandidate(const FCombatActionRequest& InIncomingRequest, FActionCandidate& OutIncomingCandidate, EActionRequestRejectReason& OutRejectReason) const
{
	OutIncomingCandidate = FActionCandidate();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!IsValid(ActionComp_Injected))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidComponent;
		return false;
	}

	FActionCandidate incomingCandidate;

	switch (InIncomingRequest.IntentType)
	{
	case ECombatActionIntent::ComboAttack:
	{
		incomingCandidate.ActionDataKey.ActionType = EActionType::ComboAttack;
		incomingCandidate.ActionDataKey.ActionIndex = ActionComp_Injected->IsActiveActionType(EActionType::ComboAttack) ? ActionComp_Injected->GetActiveActionIndex() + 1 : 0;
		break;
	}

	case ECombatActionIntent::Dodge:
	{
		incomingCandidate.ActionDataKey.ActionType = EActionType::Dodge;
		incomingCandidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	case ECombatActionIntent::Guard:
	{
		incomingCandidate.ActionDataKey.ActionType = EActionType::Guard;

		switch (InIncomingRequest.IntentEvent)
		{
		case EActionIntentEvent::Started:
		{
			incomingCandidate.ActionDataKey.ActionIndex = GetGuardActionPhaseIndex(EGuardActionPhase::In);
			break;
		}

		case EActionIntentEvent::Completed:
		{
			incomingCandidate.ActionDataKey.ActionIndex = GetGuardActionPhaseIndex(EGuardActionPhase::Out);
			break;
		}

		default:
		{
			OutRejectReason = EActionRequestRejectReason::InvalidRequest;
			return false;
		}
		}

		break;
	}

	default:
		OutRejectReason = EActionRequestRejectReason::InvalidCombatAction;
		return false;
	}

	OutIncomingCandidate = incomingCandidate;
	return true;
}

// Request Side Effects

void UCActionOrchestratorComponent::ApplyCombatActionInputSideEffects(const FCombatActionRequest& InIncomingRequest) const
{
	if (!IsValid(ActionComp_Injected)) return;
	if (InIncomingRequest.IntentType != ECombatActionIntent::Guard) return;

	switch (InIncomingRequest.IntentEvent)
	{
	case EActionIntentEvent::Started:
	{
		ActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardInputPressed));
		break;
	}

	case EActionIntentEvent::Completed:
	{
		ActionComp_Injected->ApplyOverlayEvent(FObservableOverlayEventContext(EObservableOverlayEventType::GuardInputReleased));
		break;
	}

	default:
		break;
	}
}

// Orchestration Pipeline

FActionRequestResult UCActionOrchestratorComponent::ProcessActionCandidate(const FActionCandidate& InIncomingCandidate)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	FActionExecutionContext incomingContext;

	if (!ResolveActionContext(InIncomingCandidate, incomingContext, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	const FExecutionDecisionQuery decisionQuery = BuildDecisionQuery(incomingContext);

	EDeferredActionConsumeKey consumeKey = EDeferredActionConsumeKey::None;
	if (TryResolveDeferredConsumeKey(InIncomingCandidate, decisionQuery, consumeKey))
	{
		return DeferActionCandidate(InIncomingCandidate, consumeKey);
	}

	const FExecutionDecisionResult decisionResult = BuildDecisionResult(decisionQuery, rejectReason);
	FActionExecutionResult executionResult = BuildActionExecutionResult(incomingContext, decisionResult, rejectReason);

	ResolveExecutionApplyMode(decisionQuery, executionResult);
	ResolveObservableOverlayGate(decisionQuery, executionResult);

	FExecutionOrchestratorDebug::RecordActionExecutionResultForAudit(OwnerCharacter_Injected, executionResult, TEXT("DecisionResolved"));
	FExecutionOrchestratorDebug::PrintActionExecutionDebug(OwnerCharacter_Injected, decisionQuery, executionResult);

	return DispatchActionDecision(executionResult);
}

// Execution Context Resolve

bool UCActionOrchestratorComponent::ResolveActionContext(const FActionCandidate& InIncomingCandidate, FActionExecutionContext& OutIncomingContext, EActionRequestRejectReason& OutRejectReason) const
{
	OutIncomingContext = FActionExecutionContext();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!InIncomingCandidate.IsValidMinimal())
	{
		OutRejectReason = EActionRequestRejectReason::InvalidRequest;
		return false;
	}

	FActionDataKey incomingActionDataKey = InIncomingCandidate.ActionDataKey;

	FActionData incomingActionData;
	if (!ResolveActionData(incomingActionDataKey, incomingActionData))
	{
		OutRejectReason = EActionRequestRejectReason::ActionDataNotFound;
		return false;
	}

	UCAction* incomingActionExecutor = ResolveActionExecutor(incomingActionData);
	if (!IsValid(incomingActionExecutor))
	{
		OutRejectReason = EActionRequestRejectReason::ActionExecutorNotFound;
		return false;
	}

	OutIncomingContext.ActionDataKey = incomingActionDataKey;
	OutIncomingContext.ActionData = incomingActionData;
	OutIncomingContext.ActionExecutor = incomingActionExecutor;

	return true;
}

bool UCActionOrchestratorComponent::ResolveActionData(const FActionDataKey& InIncomingDataKey, FActionData& OutIncomingData) const
{
	OutIncomingData = FActionData();

	if (!IsValid(ActionComp_Injected)) return false;
	if (!InIncomingDataKey.IsValidMinimal()) return false;

	return ActionComp_Injected->ResolveActionData(InIncomingDataKey, OutIncomingData);
}

UCAction* UCActionOrchestratorComponent::ResolveActionExecutor(const FActionData& InIncomingData) const
{
	if (!IsValid(ActionComp_Injected)) return nullptr;

	return ActionComp_Injected->ResolveActionExecutor(InIncomingData);
}

// Decision Query Build

FExecutionDecisionQuery UCActionOrchestratorComponent::BuildDecisionQuery(const FActionExecutionContext& InIncomingContext) const
{
	FExecutionDecisionQuery query;

	query.Snapshot = BuildSnapshot();
	query.IncomingPart = BuildIncomingActionParticipant(InIncomingContext);
	query.ActivePart = BuildActiveExecutionParticipant();

	return query;
}

FExecutionSnapshot UCActionOrchestratorComponent::BuildSnapshot() const
{
	FExecutionSnapshot snapshot;

	snapshot.ExecutionState = IsValid(StateComp_Injected) ? StateComp_Injected->GetCurrentExecutionState() : EExecutionState::Dead;
	snapshot.bIsDead = !IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive();

	if (IsValid(ObservableOverlayComp_Injected))
	{
		ObservableOverlayComp_Injected->WriteOverlaySnapshot(snapshot.ObservableOverlay);
	}

	return snapshot;
}

FExecutionParticipant UCActionOrchestratorComponent::BuildIncomingActionParticipant(const FActionExecutionContext& InIncomingContext) const
{
	FExecutionParticipant participant;

	if (!InIncomingContext.IsValidMinimal()) return participant;

	participant.bIsValid = true;
	participant.ParticipantDomain = EExecutionDomain::Action;
	participant.ActionContext = InIncomingContext;

	return participant;
}

FExecutionParticipant UCActionOrchestratorComponent::BuildActiveExecutionParticipant() const
{
	FExecutionParticipant participant;

	const bool bHasActiveAction = IsValid(ActionComp_Injected) && ActionComp_Injected->IsActive();
	const bool bHasActiveReaction = IsValid(ReactionComp_Injected) && ReactionComp_Injected->IsActive();

	if (bHasActiveAction && bHasActiveReaction)
	{
		FExecutionOrchestratorDebug::RecordInvalidActiveParticipantsForAudit(OwnerCharacter_Injected, TEXT("ActionOrchestrator"));
		return participant;
	}

	// 01. Active Reaction Case
	if (bHasActiveReaction)
	{
		FReactionData activeData;

		if (ReactionComp_Injected->GetActiveReactionData(activeData))
		{
			FReactionExecutionContext context;

			context.ReactionDataKey = activeData.ReactionDataKey;
			context.ReactionData = activeData;
			context.ReactionExecutor = ReactionComp_Injected->GetActiveReactionExecutor();

			if (context.IsValidMinimal())
			{
				participant.bIsValid = true;
				participant.ParticipantDomain = EExecutionDomain::Reaction;
				participant.ReactionContext = context;

				return participant;
			}
		}
	}

	// 02. Active Action Case
	if (bHasActiveAction)
	{
		FActionData activeData;

		if (ActionComp_Injected->GetActiveActionData(activeData))
		{
			FActionExecutionContext context;

			context.ActionDataKey = activeData.ActionDataKey;
			context.ActionData = activeData;
			context.ActionExecutor = ActionComp_Injected->GetActiveActionExecutor();

			if (context.IsValidMinimal())
			{
				participant.bIsValid = true;
				participant.ParticipantDomain = EExecutionDomain::Action;
				participant.ActionContext = context;

				return participant;
			}
		}
	}

	return participant;
}

// Deferred Resolve

bool UCActionOrchestratorComponent::TryResolveDeferredConsumeKey(const FActionCandidate& InIncomingCandidate, const FExecutionDecisionQuery& InQuery, EDeferredActionConsumeKey& OutConsumeKey) const
{
	OutConsumeKey = EDeferredActionConsumeKey::None;

	if (!InIncomingCandidate.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;
	if (!IsValid(InQuery.IncomingPart.GetActionContext().ActionExecutor)) return false;

	return InQuery.IncomingPart.GetActionContext().ActionExecutor->TryResolveDeferredConsumeKey(InQuery, OutConsumeKey);
}

FActionRequestResult UCActionOrchestratorComponent::DeferActionCandidate(const FActionCandidate& InIncomingCandidate, EDeferredActionConsumeKey InConsumeKey)
{
	if (!InIncomingCandidate.IsValidMinimal())
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidRequest);

	if (InConsumeKey == EDeferredActionConsumeKey::None || InConsumeKey == EDeferredActionConsumeKey::Max)
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidRequest);

	FDeferredActionCandidate deferredCandidate;
	deferredCandidate.Candidate = InIncomingCandidate;
	deferredCandidate.ConsumeKey = InConsumeKey;

	if (!deferredCandidate.IsValidMinimal())
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidRequest);

	// Clear the same deferred candidate before storing the latest one.
	DeferredActionCandidates.RemoveAll(
		[InConsumeKey, InIncomingCandidate](const FDeferredActionCandidate& InEntry)
		{
			return InEntry.MatchesIdentity(InConsumeKey, InIncomingCandidate);
		});

	DeferredActionCandidates.Add(deferredCandidate);

	return BuildActionRequestResult(EActionRequestResultType::Deferred);
}

// Decision Build

FExecutionDecisionResult UCActionOrchestratorComponent::BuildDecisionResult(const FExecutionDecisionQuery& InQuery, EActionRequestRejectReason& OutRejectReason) const
{
	FExecutionDecisionResult result;
	OutRejectReason = EActionRequestRejectReason::None;

	if (!InQuery.HasIncomingPart())
	{
		result.Decision = EExecutionDecision::Reject;
		OutRejectReason = EActionRequestRejectReason::InvalidQuery;

		return result;
	}

	if (!InQuery.IncomingPart.IsActionParticipant())
	{
		result.Decision = EExecutionDecision::Reject;
		OutRejectReason = EActionRequestRejectReason::InvalidQuery;

		return result;
	}

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();
	UCAction* incomingExecutor = incomingContext.ActionExecutor;

	if (!IsValid(incomingExecutor))
	{
		result.Decision = EExecutionDecision::Reject;
		OutRejectReason = EActionRequestRejectReason::ActionExecutorNotFound;

		return result;
	}

	result = incomingExecutor->ResolveExecutionDecision(InQuery);

	if (result.Decision == EExecutionDecision::Reject)
	{
		OutRejectReason = EActionRequestRejectReason::RejectedByExecutor;
	}

	return result;
}

FActionExecutionResult UCActionOrchestratorComponent::BuildActionExecutionResult(const FActionExecutionContext& InContext, const FExecutionDecisionResult& InDecisionResult, EActionRequestRejectReason InRejectReason) const
{
	FActionExecutionResult result;

	result.Decision = InDecisionResult.Decision;
	result.Relationship = InDecisionResult.Relationship;
	result.ApplyMode = EExecutionApplyMode::None;
	result.ResolvedContext = InContext;
	result.RejectReason = InRejectReason;

	return result;
}

// Decision Refinement

void UCActionOrchestratorComponent::ResolveExecutionApplyMode(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const
{
	InOutResult.ApplyMode = EExecutionApplyMode::None;
	InOutResult.InterventionDirective = FExecutionInterventionDirective();

	if (!InOutResult.IsAcceptedDecision()) return;

	switch (InOutResult.Relationship)
	{
	case EExecutionRelationship::Independent:
	{
		if (!(InQuery.Snapshot.IsIdle() && !InQuery.HasActivePart()))
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EActionRequestRejectReason::InvalidIndependent;
			return;
		}

		InOutResult.ApplyMode = EExecutionApplyMode::Start;
		return;
	}

	case EExecutionRelationship::Sequential:
	{
		if (!(InQuery.Snapshot.IsInAction() && InQuery.HasActivePart()))
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EActionRequestRejectReason::InvalidSequential;
			return;
		}

		// Sequential action is reserved and consumed later by notify timing.
		InOutResult.ApplyMode = EExecutionApplyMode::Reserve;
		return;
	}

	case EExecutionRelationship::Exclusive:
	{
		if (InQuery.Snapshot.IsIdle() || !InQuery.HasActivePart())
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EActionRequestRejectReason::InvalidExclusive;
			return;
		}

		ResolveInterventionDirective(InQuery, InOutResult);

		if (!InOutResult.IsAcceptedDecision()) return;

		if (!InOutResult.InterventionDirective.IsRequested())
		{
			InOutResult.Decision = EExecutionDecision::Reject;
			InOutResult.RejectReason = EActionRequestRejectReason::InvalidExclusive;
			return;
		}

		InOutResult.ApplyMode = EExecutionApplyMode::Intervene;
		return;
	}

	default:
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EActionRequestRejectReason::NoExecutableAction;
		return;
	}
}

void UCActionOrchestratorComponent::ResolveInterventionDirective(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const
{
	InOutResult.InterventionDirective = FExecutionInterventionDirective();

	if (!InOutResult.IsAcceptedDecision()) return;
	if (!InQuery.HasActivePart()) return;

	FExecutionInterventionQuery interventionQuery;

	if (!BuildInterventionQuery(InQuery, EExecutionStopReason::Interrupted, interventionQuery))
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EActionRequestRejectReason::InvalidQuery;
		return;
	}

	const FExecutionParticipant& incoming = interventionQuery.IncomingPart;
	const FExecutionParticipant& active = interventionQuery.ActivePart;

	bool bIncomingWants = false;
	bool bActiveAllows = false;

	UCAction* incomingAction = Cast<UCAction>(incoming.GetExecutor());
	if (!IsValid(incomingAction))
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EActionRequestRejectReason::IncomingCannotIntervene;
		return;
	}

	bIncomingWants = incomingAction->WantIntervention(interventionQuery);

	if (UCAction* activeAction = Cast<UCAction>(active.GetExecutor()))
	{
		bActiveAllows = activeAction->AllowIntervention(interventionQuery);
	}
	else if (UCReaction* activeReaction = Cast<UCReaction>(active.GetExecutor()))
	{
		bActiveAllows = activeReaction->AllowIntervention(interventionQuery);
	}

	if (!bActiveAllows || !bIncomingWants)
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = !bActiveAllows
			? EActionRequestRejectReason::ActiveCannotAcceptIntervention
			: EActionRequestRejectReason::IncomingCannotIntervene;
		return;
	}

	FExecutionInterventionDirective directive;

	if (!BuildInterventionDirective(interventionQuery, EExecutionStopSource::ActionOrchestration, EExecutionAfterStopAction::StartIncoming, directive))
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = EActionRequestRejectReason::InterventionDispatchFailed;
		return;
	}

	InOutResult.InterventionDirective = directive;
}

void UCActionOrchestratorComponent::ResolveObservableOverlayGate(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const
{
	InOutResult.OverlayHandlings.Empty();

	if (!InOutResult.IsAcceptedDecision()) return;

	const bool bNeedsExecutionStart = InOutResult.ApplyMode == EExecutionApplyMode::Start || InOutResult.ApplyMode == EExecutionApplyMode::Intervene;
	if (!bNeedsExecutionStart) return;

	FObservableOverlayQuery overlayQuery;
	overlayQuery.DecisionQuery = InQuery;
	overlayQuery.ApplyMode = InOutResult.ApplyMode;

	if (InQuery.IncomingPart.IsActionParticipant())
	{
		if (const UCAction* incomingAction = InQuery.IncomingPart.GetActionContext().ActionExecutor)
		{
			FObservableOverlayExecutionDecision overlayDecision;
			incomingAction->ResolveObservableOverlayCondition(overlayQuery, overlayDecision);

			if (!overlayDecision.IsAccepted())
			{
				InOutResult.Decision = overlayDecision.Decision;
				InOutResult.RejectReason = EActionRequestRejectReason::RejectedByOverlay;
				return;
			}

			for (const EObservableOverlayHandling handling : overlayDecision.Handlings)
			{
				if (handling == EObservableOverlayHandling::None) continue;

				InOutResult.OverlayHandlings.AddUnique(handling);
			}
		}
	}
}

// Intervention Build

bool UCActionOrchestratorComponent::BuildInterventionQuery(const FExecutionDecisionQuery& InQuery, EExecutionStopReason InStopReason, FExecutionInterventionQuery& OutQuery) const
{
	OutQuery = FExecutionInterventionQuery();

	if (!InQuery.IncomingPart.IsValidMinimal()) return false;
	if (!InQuery.ActivePart.IsValidMinimal()) return false;

	if (InStopReason == EExecutionStopReason::None || InStopReason == EExecutionStopReason::Max) return false;

	OutQuery.Snapshot = InQuery.Snapshot;
	OutQuery.IncomingPart = InQuery.IncomingPart;
	OutQuery.ActivePart = InQuery.ActivePart;
	OutQuery.StopReason = InStopReason;

	return OutQuery.IsValidMinimal();
}

bool UCActionOrchestratorComponent::BuildInterventionDirective(const FExecutionInterventionQuery& InQuery, EExecutionStopSource InStopSource, EExecutionAfterStopAction InAfterStopAction, FExecutionInterventionDirective& OutDirective) const
{
	OutDirective = FExecutionInterventionDirective();

	if (!InQuery.IsValidMinimal()) return false;

	if (InStopSource == EExecutionStopSource::None || InStopSource == EExecutionStopSource::Max) return false;
	if (InAfterStopAction == EExecutionAfterStopAction::None || InAfterStopAction == EExecutionAfterStopAction::Max) return false;

	OutDirective.bRequested = true;
	OutDirective.StopSource = InStopSource;
	OutDirective.SourceDomain = InQuery.IncomingPart.ParticipantDomain;
	OutDirective.TargetDomain = InQuery.ActivePart.ParticipantDomain;
	OutDirective.StopReason = InQuery.StopReason;
	OutDirective.AfterStopAction = InAfterStopAction;
	OutDirective.IncomingPart = InQuery.IncomingPart;
	OutDirective.ActivePart = InQuery.ActivePart;

	return OutDirective.IsValidRequest();
}

// Decision Dispatch

FActionRequestResult UCActionOrchestratorComponent::DispatchActionDecision(const FActionExecutionResult& InResult)
{
	if (InResult.Decision == EExecutionDecision::Ignore)
		return BuildActionRequestResult(EActionRequestResultType::Ignored);

	if (!InResult.IsAcceptedDecision())
		return BuildActionRequestResult(EActionRequestResultType::Rejected, InResult.RejectReason);

	if (!IsValid(ActionComp_Injected))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!ActionComp_Injected->ApplyActionDecision(InResult))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::ActionExecutionFailed);

	EActionRequestResultType resultType = ConvertDecisionToResultType(InResult);

	return BuildActionRequestResult(resultType);
}

// Result Build

EActionRequestResultType UCActionOrchestratorComponent::ConvertDecisionToResultType(const FActionExecutionResult& InResult) const
{
	if (InResult.Decision == EExecutionDecision::Reject)
		return EActionRequestResultType::Rejected;

	if (InResult.Decision == EExecutionDecision::Ignore)
		return EActionRequestResultType::Ignored;

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
		return EActionRequestResultType::Started;

	case EExecutionApplyMode::Reserve:
		return EActionRequestResultType::Reserved;

	case EExecutionApplyMode::Intervene:
		return EActionRequestResultType::Intervened;

	default:
		return EActionRequestResultType::None;
	}
}

FActionRequestResult UCActionOrchestratorComponent::BuildActionRequestResult(EActionRequestResultType InResultType, EActionRequestRejectReason InRejectReason) const
{
	FActionRequestResult result;

	result.ResultType = InResultType;
	result.RejectReason = InRejectReason;

	if (InResultType == EActionRequestResultType::Rejected)
	{
		result.RejectReason = (InRejectReason != EActionRequestRejectReason::None) ? InRejectReason : EActionRequestRejectReason::NoExecutableAction;
	}
	else
	{
		result.RejectReason = EActionRequestRejectReason::None;
	}

	FExecutionOrchestratorDebug::RecordActionRequestResultForAudit(OwnerCharacter_Injected, result, TEXT("RequestResult"));

	return result;
}
