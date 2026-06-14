#include "Component/CActionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"

#include "Action/CAction.h"
#include "Reaction/CReaction.h"

#include "Type/CActionOrchestrationStructure.h"

UCActionOrchestratorComponent::UCActionOrchestratorComponent()
{
}

void UCActionOrchestratorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	MovementComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCMovementComponent>();
	WeaponComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCWeaponComponent>();
	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	ActionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCActionComponent>();
	ReactionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCReactionComponent>();
}

FActionRequestResult UCActionOrchestratorComponent::RequestMovementAction(const FMovementActionRequest& InIncomingRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	// Release-style cleanup is allowed before hard-block checks.
	if (InIncomingRequest.IntentType == EMovementActionIntent::StopJump)
	{
		MovementComp_Cached->OnStopJump();
		return BuildActionRequestResult(EActionRequestResultType::Handled);
	}

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	const EExecutionState executionState = IsValid(StateComp_Cached) ? StateComp_Cached->GetCurrentExecutionState() : EExecutionState::Dead;

	if (executionState == EExecutionState::Reaction)
		return BuildActionRequestResult(EActionRequestResultType::Ignored);

	switch (InIncomingRequest.IntentType)
	{
	case EMovementActionIntent::Move:
	{
		MovementComp_Cached->OnMove(InIncomingRequest.Axis2D);
		break;
	}
	case EMovementActionIntent::Walk:
	{
		MovementComp_Cached->OnWalk();
		break;
	}
	case EMovementActionIntent::Run:
	{
		MovementComp_Cached->OnRun();
		break;
	}
	case EMovementActionIntent::Sprint:
	{
		MovementComp_Cached->OnSprint();
		break;
	}
	case EMovementActionIntent::Jump:
	{
		MovementComp_Cached->OnJump();
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

	if (!IsValid(WeaponComp_Cached) || !IsValid(ActionComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	FActionCandidate incomingCandidate;

	if (!ResolveEquipmentActionCandidate(InIncomingRequest, incomingCandidate, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	return ExecuteActionCandidate(incomingCandidate);
}

FActionRequestResult UCActionOrchestratorComponent::RequestCombatAction(const FCombatActionRequest& InIncomingRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(ActionComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	FActionCandidate incomingCandidate;

	if (!ResolveCombatActionCandidate(InIncomingRequest, incomingCandidate, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	return ExecuteActionCandidate(incomingCandidate);
}

bool UCActionOrchestratorComponent::CanAcceptActionRequest(EActionRequestRejectReason& OutRejectReason) const
{
	OutRejectReason = EActionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Cached))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidOwner;
		return false;
	}

	if (!IsValid(HealthComp_Cached) || !IsValid(StateComp_Cached))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidComponent;
		return false;
	}

	if (!HealthComp_Cached->IsAlive())
	{
		OutRejectReason = EActionRequestRejectReason::Dead;
		return false;
	}

	const EExecutionState executionState = StateComp_Cached->GetCurrentExecutionState();

	if (executionState == EExecutionState::Dead)
	{
		OutRejectReason = EActionRequestRejectReason::Dead;
		return false;
	}

	return true;
}

bool UCActionOrchestratorComponent::ResolveEquipmentActionCandidate(const FEquipmentActionRequest& InIncomingRequest, FActionCandidate& OutIncomingCandidate, EActionRequestRejectReason& OutRejectReason) const
{
	OutIncomingCandidate = FActionCandidate();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!IsValid(WeaponComp_Cached))
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
		incomingCandidate.ActionDataKey.ActionType = WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed) ? EActionType::Equip : EActionType::Unequip;
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

	if (!IsValid(ActionComp_Cached))
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
		incomingCandidate.ActionDataKey.ActionIndex = ActionComp_Cached->IsActiveActionType(EActionType::ComboAttack) ? ActionComp_Cached->GetActiveActionIndex() + 1 : 0;
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
			// Temporary: Started -> Guard index 1 -> Block_In
			incomingCandidate.ActionDataKey.ActionIndex = 1;
			break;
		}

		case EActionIntentEvent::Completed:
		{
			// Temporary: Completed -> Guard index 2 -> Block_Out
			incomingCandidate.ActionDataKey.ActionIndex = 2;
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

FActionRequestResult UCActionOrchestratorComponent::ExecuteActionCandidate(const FActionCandidate& InIncomingCandidate)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	FActionExecutionContext incomingContext;

	if (!ResolveActionContext(InIncomingCandidate, incomingContext, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	const FExecutionDecisionQuery decisionQuery = BuildDecisionQuery(incomingContext);
	const FExecutionDecisionResult decisionResult = BuildDecisionResult(decisionQuery, rejectReason);
	FActionExecutionResult executionResult = BuildActionExecutionResult(incomingContext, decisionResult, rejectReason);

	ResolveExecutionApplyMode(decisionQuery, executionResult);

	return DispatchActionDecision(executionResult);
}

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

	if (!IsValid(ActionComp_Cached)) return false;
	if (!InIncomingDataKey.IsValidMinimal()) return false;

	return ActionComp_Cached->ResolveActionData(InIncomingDataKey, OutIncomingData);
}

UCAction* UCActionOrchestratorComponent::ResolveActionExecutor(const FActionData& InIncomingData) const
{
	if (!IsValid(ActionComp_Cached)) return nullptr;

	return ActionComp_Cached->ResolveActionExecutor(InIncomingData);
}

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

	snapshot.ExecutionState = IsValid(StateComp_Cached) ? StateComp_Cached->GetCurrentExecutionState() : EExecutionState::Dead;
	snapshot.bIsDead = !IsValid(HealthComp_Cached) || !HealthComp_Cached->IsAlive();

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

	const bool bHasActiveAction = IsValid(ActionComp_Cached) && ActionComp_Cached->IsActive();
	const bool bHasActiveReaction = IsValid(ReactionComp_Cached) && ReactionComp_Cached->IsActive();

	if (bHasActiveAction && bHasActiveReaction)
	{
		FLog::Log(TEXT("[ActionOrchestrator] Invalid execution state (action and reaction are both active)."));
		return participant;
	}

	// 01. Active Reaction Case
	if (bHasActiveReaction)
	{
		FReactionData activeData;

		if (ReactionComp_Cached->GetActiveReactionData(activeData))
		{
			FReactionExecutionContext context;

			context.ReactionDataKey = activeData.ReactionDataKey;
			context.ReactionData = activeData;
			context.ReactionExecutor = ReactionComp_Cached->GetActiveReactionExecutor();

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

		if (ActionComp_Cached->GetActiveActionData(activeData))
		{
			FActionExecutionContext context;

			context.ActionDataKey = activeData.ActionDataKey;
			context.ActionData = activeData;
			context.ActionExecutor = ActionComp_Cached->GetActiveActionExecutor();

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

void UCActionOrchestratorComponent::ResolveExecutionApplyMode(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const
{
	InOutResult.ApplyMode = EExecutionApplyMode::None;
	InOutResult.InterventionDirective = FExecutionInterventionDirective();

	// [NOTE] Early return ignore and reject decision
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

	if (!bIncomingWants || !bActiveAllows)
	{
		InOutResult.Decision = EExecutionDecision::Reject;
		InOutResult.RejectReason = !bIncomingWants
			? EActionRequestRejectReason::IncomingCannotIntervene
			: EActionRequestRejectReason::ActiveCannotAcceptIntervention;
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

	return OutDirective.IsValidRequest();
}

FActionRequestResult UCActionOrchestratorComponent::DispatchActionDecision(const FActionExecutionResult& InResult)
{
	// [NOTE] Request ignore result
	if (InResult.Decision == EExecutionDecision::Ignore)
		return BuildActionRequestResult(EActionRequestResultType::Ignored);

	// [NOTE] Request rejected result
	if (!InResult.IsAcceptedDecision())
		return BuildActionRequestResult(EActionRequestResultType::Rejected, InResult.RejectReason);

	if (!IsValid(ActionComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!ActionComp_Cached->ApplyActionDecision(InResult))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::ActionExecutionFailed);

	EActionRequestResultType resultType = ConvertDecisionToResultType(InResult);

	return BuildActionRequestResult(resultType);
}

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
		PrintActionRequestResult(result);
	}
	else
	{
		result.RejectReason = EActionRequestRejectReason::None;
	}

	return result;
}

void UCActionOrchestratorComponent::PrintActionRequestResult(const FActionRequestResult& InResult) const
{
	FLog::Log(FString::Printf(
		TEXT("[ActionRequestResult] Owner = %s | ResultType = %s | RejectReason = %s"),
		*GetNameSafe(OwnerCharacter_Cached),
		*UEnum::GetValueAsString(InResult.ResultType),
		*UEnum::GetValueAsString(InResult.RejectReason)
	));
}
