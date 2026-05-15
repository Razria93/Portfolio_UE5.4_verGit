#include "Component/CActionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CHealthComponent.h"

#include "Action/CAction.h"

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
	ActionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCActionComponent>();
	ReactionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCReactionComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
}

FActionRequestResult UCActionOrchestratorComponent::RequestMovementAction(const FMovementActionRequest& InRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	// Release-style cleanup is allowed before hard-block checks.
	if (InRequest.IntentType == EMovementActionIntent::StopJump)
	{
		MovementComp_Cached->OnStopJump();
		return BuildActionRequestResult(EActionRequestResultType::Handled);
	}

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	const EExecutionState executionState = IsValid(StateComp_Cached) ? StateComp_Cached->GetCurrentExecutionState() : EExecutionState::Dead;

	if (executionState == EExecutionState::Reaction)		
		return BuildActionRequestResult(EActionRequestResultType::Ignored);

	switch (InRequest.IntentType)
	{
	case EMovementActionIntent::Move:
	{
		MovementComp_Cached->OnMove(InRequest.Axis2D);
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

FActionRequestResult UCActionOrchestratorComponent::RequestEquipmentAction(const FEquipmentActionRequest& InRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(WeaponComp_Cached) || !IsValid(ActionComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	FActionCandidate candidate;

	if (!ResolveEquipmentActionCandidate(InRequest, candidate, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	return ExecuteActionCandidate(InRequest.IntentSource, candidate);
}

FActionRequestResult UCActionOrchestratorComponent::RequestCombatAction(const FCombatActionRequest& InRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	if (!IsValid(ActionComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!CanAcceptActionRequest(rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	FActionCandidate candidate;

	if (!ResolveCombatActionCandidate(InRequest, candidate, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	return ExecuteActionCandidate(InRequest.IntentSource, candidate);
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

bool UCActionOrchestratorComponent::ResolveEquipmentActionCandidate(const FEquipmentActionRequest& InRequest, FActionCandidate& OutCandidate, EActionRequestRejectReason& OutRejectReason) const
{
	OutCandidate = FActionCandidate();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!IsValid(WeaponComp_Cached))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidComponent;
		return false;
	}

	FActionCandidate candidate;

	switch (InRequest.IntentType)
	{
	case EEquipmentActionIntent::Equip:
	{
		candidate.ActionDataKey.ActionType = EActionType::Equip;
		candidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	case EEquipmentActionIntent::Unequip:
	{
		candidate.ActionDataKey.ActionType = EActionType::Unequip;
		candidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	case EEquipmentActionIntent::Toggle:
	{
		candidate.ActionDataKey.ActionType = WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed) ? EActionType::Equip : EActionType::Unequip;
		candidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	default:
		OutRejectReason = EActionRequestRejectReason::InvalidEquipment;
		return false;
	}

	OutCandidate = candidate;
	return true;
}

bool UCActionOrchestratorComponent::ResolveCombatActionCandidate(const FCombatActionRequest& InRequest, FActionCandidate& OutCandidate, EActionRequestRejectReason& OutRejectReason) const
{
	OutCandidate = FActionCandidate();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!IsValid(ActionComp_Cached))
	{
		OutRejectReason = EActionRequestRejectReason::InvalidComponent;
		return false;
	}

	FActionCandidate candidate;

	switch (InRequest.IntentType)
	{
	case ECombatActionIntent::ComboAttack:
	{
		candidate.ActionDataKey.ActionType = EActionType::ComboAttack;
		candidate.ActionDataKey.ActionIndex = ActionComp_Cached->IsActiveActionType(EActionType::ComboAttack) ? ActionComp_Cached->GetActiveActionIndex() + 1 : 0;
		break;
	}

	case ECombatActionIntent::Dodge:
	{
		candidate.ActionDataKey.ActionType = EActionType::Dodge;
		candidate.ActionDataKey.ActionIndex = 0;
		break;
	}

	default:
		OutRejectReason = EActionRequestRejectReason::InvalidCombatAction;
		return false;
	}

	OutCandidate = candidate;
	return true;
}

FActionRequestResult UCActionOrchestratorComponent::ExecuteActionCandidate(EActionIntentSource InSource, const FActionCandidate& InCandidate)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;

	FActionResolvedContext context = FActionResolvedContext();

	if (!ResolveActionContext(InCandidate, context, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	FActionLocalLevelQuery localQuery = BuildLocalLevelQuery(context);
	FActionLocalLevelResult localResult = ResolveLocalLevelResult(localQuery);

	if (localResult.Decision == EActionLocalLevelDecision::Ignore)
		return BuildActionRequestResult(EActionRequestResultType::Ignored, EActionRequestRejectReason::None);

	if (!localResult.IsAcceptedDecision())
		return BuildActionRequestResult(EActionRequestResultType::Rejected, localResult.RejectReason);

	FActionResolvedPolicy policy;

	if (!ResolveActionPolicy(localQuery, localResult, policy, rejectReason))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, rejectReason);

	FActionOrchestrationLevelQuery orchestrationQuery = BuildOrchestrationLevelQuery(InSource, localQuery, localResult, policy);
	FActionOrchestrationLevelResult orchestrationResult = ResolveOrchestrationLevelResult(orchestrationQuery);

	ResolveExecutionInterventionDirective(orchestrationResult);

	return DispatchActionDecision(orchestrationResult);
}

bool UCActionOrchestratorComponent::ResolveActionContext(const FActionCandidate& InCandidate, FActionResolvedContext& OutContext, EActionRequestRejectReason& OutRejectReason) const
{
	OutContext = FActionResolvedContext();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!InCandidate.IsValidMinimal())
	{
		OutRejectReason = EActionRequestRejectReason::InvalidRequest;
		return false;
	}

	FActionDataKey actionDataKey = InCandidate.ActionDataKey;

	FActionData actionData;
	if (!ResolveActionData(actionDataKey, actionData))
	{
		OutRejectReason = EActionRequestRejectReason::ActionDataNotFound;
		return false;
	}

	UCAction* actionExecutor = ResolveActionExecutor(actionData);
	if (!IsValid(actionExecutor))
	{
		OutRejectReason = EActionRequestRejectReason::ActionExecutorNotFound;
		return false;
	}

	OutContext.ActionDataKey = actionDataKey;
	OutContext.ActionData = actionData;
	OutContext.ActionExecutor = actionExecutor;

	return true;
}

bool UCActionOrchestratorComponent::ResolveActionData(const FActionDataKey& InDataKey, FActionData& OutData) const
{
	OutData = FActionData();

	if (!IsValid(ActionComp_Cached)) return false;
	if (!InDataKey.IsValidExactKey()) return false;

	return ActionComp_Cached->ResolveActionData(InDataKey, OutData);
}

UCAction* UCActionOrchestratorComponent::ResolveActionExecutor(const FActionData& InData) const
{
	if (!IsValid(ActionComp_Cached)) return nullptr;

	return ActionComp_Cached->ResolveActionExecutor(InData);
}

FActionLocalLevelQuery UCActionOrchestratorComponent::BuildLocalLevelQuery(const FActionResolvedContext& InIncoming) const
{
	FActionLocalLevelQuery query;

	query.ExecutionState = IsValid(StateComp_Cached) ? StateComp_Cached->GetCurrentExecutionState() : EExecutionState::Dead;
	query.IncomingContext = InIncoming;

	if (IsValid(ActionComp_Cached))
	{
		query.bIsActiveAction = ActionComp_Cached->IsActive();

		if (query.bIsActiveAction)
		{
			FActionData activeData;
			if (ActionComp_Cached->GetActiveActionData(activeData))
			{
				query.ActiveContext.ActionDataKey = activeData.ActionDataKey;
				query.ActiveContext.ActionData = activeData;
				query.ActiveContext.ActionExecutor = ActionComp_Cached->GetActiveActionExecutor();
			}
		}
	}

	return query;
}

FActionLocalLevelResult UCActionOrchestratorComponent::ResolveLocalLevelResult(const FActionLocalLevelQuery& InLocalQuery) const
{
	FActionLocalLevelResult result;

	if (!InLocalQuery.IncomingContext.IsValidMinimal())
	{
		result.Decision = EActionLocalLevelDecision::Reject;
		result.RejectReason = EActionRequestRejectReason::NoExecutableAction;

		return result;
	}

	UCAction* incomingExecutor = InLocalQuery.IncomingContext.ActionExecutor;
	if (!IsValid(incomingExecutor))
	{
		result.Decision = EActionLocalLevelDecision::Reject;
		result.RejectReason = EActionRequestRejectReason::ActionExecutorNotFound;

		return result;
	}

	result.Decision = incomingExecutor->ResolveLocalLevelDecision(InLocalQuery);

	if (result.Decision == EActionLocalLevelDecision::Reject)
	{
		result.RejectReason = EActionRequestRejectReason::NoExecutableAction;
	}

	return result;
}

bool UCActionOrchestratorComponent::ResolveActionPolicy(const FActionLocalLevelQuery& InLocalQuery, const FActionLocalLevelResult& InLocalResult, FActionResolvedPolicy& OutPolicy, EActionRequestRejectReason& OutRejectReason) const
{
	OutPolicy = FActionResolvedPolicy();
	OutRejectReason = EActionRequestRejectReason::None;

	if (!InLocalQuery.IncomingContext.IsValidMinimal())
	{
		OutRejectReason = EActionRequestRejectReason::NoExecutableAction;
		return false;
	}

	if (!InLocalResult.IsAcceptedDecision())
	{
		OutRejectReason = InLocalResult.RejectReason;
		return false;
	}

	const EExecutionState executionState = InLocalQuery.ExecutionState;

	const bool bIsIdleState = executionState == EExecutionState::Idle;
	const bool bIsActionState = executionState == EExecutionState::Action;
	const bool bIsReactionState = executionState == EExecutionState::Reaction;

	const bool bIsActiveAction = InLocalQuery.bIsActiveAction;
	const bool bHasActiveActionContext = InLocalQuery.ActiveContext.IsValidMinimal();

	const bool bIsActiveReaction = IsValid(ReactionComp_Cached) && ReactionComp_Cached->IsActive();

	switch (InLocalResult.Decision)
	{
	case EActionLocalLevelDecision::Start:
	{
		OutPolicy.bCanStart = bIsIdleState && !bIsActiveAction && !bIsActiveReaction;
		break;
	}

	case EActionLocalLevelDecision::Chain:
	{
		OutPolicy.bCanChain = bIsActionState && bIsActiveAction && bHasActiveActionContext;
		break;
	}

	case EActionLocalLevelDecision::Enqueue:
	{
		OutPolicy.bCanEnqueue = false;
		break;
	}

	case EActionLocalLevelDecision::Interrupt:
	{
		OutPolicy.bCanInterrupt = bIsActionState && bIsActiveAction && bHasActiveActionContext;
		break;
	}

	case EActionLocalLevelDecision::Cancel:
	{
		OutPolicy.bCanCancel = !bIsActiveAction && bIsReactionState && bIsActiveReaction;
		break;
	}

	default:
		OutRejectReason = EActionRequestRejectReason::NoExecutableAction;
		return false;
	}

	return true;
}

FActionOrchestrationLevelQuery UCActionOrchestratorComponent::BuildOrchestrationLevelQuery(EActionIntentSource InSource, const FActionLocalLevelQuery& InLocalQuery, const FActionLocalLevelResult& InLocalResult, const FActionResolvedPolicy& InPolicy) const
{
	FActionOrchestrationLevelQuery query;

	query.IntentSource = InSource;
	query.IncomingContext = InLocalQuery.IncomingContext;
	query.ActiveContext = InLocalQuery.ActiveContext;
	query.LocalLevelResult = InLocalResult;
	query.ResolvedPolicy = InPolicy;

	return query;
}

FActionOrchestrationLevelResult UCActionOrchestratorComponent::ResolveOrchestrationLevelResult(const FActionOrchestrationLevelQuery& InQuery) const
{
	FActionOrchestrationLevelResult result;

	result.ResolvedContext = InQuery.IncomingContext;

	switch (InQuery.LocalLevelResult.Decision)
	{
	case EActionLocalLevelDecision::Start:
	{
		result.Decision = InQuery.ResolvedPolicy.bCanStart
			? EActionOrchestrationLevelDecision::Start
			: EActionOrchestrationLevelDecision::Reject;
		break;
	}

	case EActionLocalLevelDecision::Chain:
	{
		result.Decision = InQuery.ResolvedPolicy.bCanChain
			? EActionOrchestrationLevelDecision::Chain
			: EActionOrchestrationLevelDecision::Reject;
		break;
	}

	case EActionLocalLevelDecision::Enqueue:
	{
		result.Decision = InQuery.ResolvedPolicy.bCanEnqueue
			? EActionOrchestrationLevelDecision::Enqueue
			: EActionOrchestrationLevelDecision::Reject;
		break;
	}

	case EActionLocalLevelDecision::Interrupt:
	{
		result.Decision = InQuery.ResolvedPolicy.bCanInterrupt
			? EActionOrchestrationLevelDecision::Interrupt
			: EActionOrchestrationLevelDecision::Reject;
		break;
	}

	case EActionLocalLevelDecision::Cancel:
	{
		result.Decision = InQuery.ResolvedPolicy.bCanCancel
			? EActionOrchestrationLevelDecision::Cancel
			: EActionOrchestrationLevelDecision::Reject;
		break;
	}

	case EActionLocalLevelDecision::Ignore:
	{
		result.Decision = EActionOrchestrationLevelDecision::Ignore;
		break;
	}

	case EActionLocalLevelDecision::Reject:
	default:
		result.Decision = EActionOrchestrationLevelDecision::Reject;
		result.RejectReason = InQuery.LocalLevelResult.RejectReason;
		break;
	}

	return result;
}

void UCActionOrchestratorComponent::ResolveExecutionInterventionDirective(FActionOrchestrationLevelResult& InOutResult) const
{
	InOutResult.InterventionDirective = FExecutionInterventionDirective();

	if (!InOutResult.IsAcceptedDecision()) return;
	if (!IsValid(ReactionComp_Cached)) return;
	if (!ReactionComp_Cached->IsActive()) return;

	const EReactionType activeReactionType = ReactionComp_Cached->GetActiveReactionType();

	// No active reaction to stop
	if (activeReactionType == EReactionType::None
		|| activeReactionType == EReactionType::Idle
		|| activeReactionType == EReactionType::All
		|| activeReactionType == EReactionType::Max)
	{
		return;
	}

	// Cannot stop if already dead
	if (activeReactionType == EReactionType::Dead)
	{
		return;
	}

	switch (InOutResult.Decision)
	{
	case EActionOrchestrationLevelDecision::Cancel:
	{
		InOutResult.InterventionDirective.bRequested = true;
		InOutResult.InterventionDirective.TargetDomain = EExecutionDomain::Reaction;	// [TODO] 어떤걸 캔슬 할 수 있는지는 Local에서 축정해야하지 않나?
		InOutResult.InterventionDirective.StopReason = EExecutionStopReason::Cancelled;
		InOutResult.InterventionDirective.StopSource = EExecutionStopSource::ActionOrchestration;
		InOutResult.InterventionDirective.AfterStopAction = EExecutionAfterStopAction::StartIncoming;
		break;
	}

	case EActionOrchestrationLevelDecision::Interrupt:
	{
		// Later: action -> action or action -> reaction interrupt.
		break;
	}

	default:
		break;
	}
}

FActionRequestResult UCActionOrchestratorComponent::DispatchActionDecision(const FActionOrchestrationLevelResult& InResult)
{
	if (!InResult.IsAcceptedDecision())
	{
		if (InResult.Decision == EActionOrchestrationLevelDecision::Ignore)
		{
			return BuildActionRequestResult(EActionRequestResultType::Ignored);
		}
		else
		{
			return BuildActionRequestResult(EActionRequestResultType::Rejected, InResult.RejectReason);
		}
	}

	if (!IsValid(ActionComp_Cached))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::InvalidComponent);

	if (!ActionComp_Cached->ApplyActionDecision(InResult))
		return BuildActionRequestResult(EActionRequestResultType::Rejected, EActionRequestRejectReason::NoExecutableAction);

	EActionRequestResultType resultType = ConvertDecisionToResultType(InResult.Decision);

	return BuildActionRequestResult(resultType);
}

EActionRequestResultType UCActionOrchestratorComponent::ConvertDecisionToResultType(EActionOrchestrationLevelDecision InDecision) const
{
	switch (InDecision)
	{
	case EActionOrchestrationLevelDecision::Handle:
		return EActionRequestResultType::Handled;

	case EActionOrchestrationLevelDecision::Start:
		return EActionRequestResultType::Started;

	case EActionOrchestrationLevelDecision::Chain:
		return EActionRequestResultType::Chained;

	case EActionOrchestrationLevelDecision::Enqueue:
		return EActionRequestResultType::Enqueued;

	case EActionOrchestrationLevelDecision::Interrupt:
		return EActionRequestResultType::Interrupted;

	case EActionOrchestrationLevelDecision::Cancel:
		return EActionRequestResultType::Cancelled;

	case EActionOrchestrationLevelDecision::Ignore:
		return EActionRequestResultType::Ignored;

	case EActionOrchestrationLevelDecision::Reject:
		return EActionRequestResultType::Rejected;

	case EActionOrchestrationLevelDecision::None:
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

	return result;
}
