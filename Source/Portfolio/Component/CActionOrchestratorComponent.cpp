#include "Component/CActionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CHealthComponent.h"

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
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
}

FActionRequestResult UCActionOrchestratorComponent::RequestMovementAction(const FMovementActionRequest& InActionRequest)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached))
		return BuildRejectedResult(EActionRequestRejectReason::InvalidComponent);

	// Release-style cleanup is allowed before hard-block checks.
	if (InActionRequest.IntentType == EMovementActionIntent::StopJump)
	{
		MovementComp_Cached->OnStopJump();
		return BuildHandledResult();
	}

	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;
	if (!CanAcceptActionRequest(rejectReason)) return BuildRejectedResult(rejectReason);

	switch (InActionRequest.IntentType)
	{
	case EMovementActionIntent::Move:
	{
		MovementComp_Cached->OnMove(InActionRequest.Axis2D);
		return BuildHandledResult();
	}

	case EMovementActionIntent::Walk:
	{
		MovementComp_Cached->OnWalk();
		return BuildHandledResult();
	}

	case EMovementActionIntent::Run:
	{
		MovementComp_Cached->OnRun();
		return BuildHandledResult();
	}

	case EMovementActionIntent::Sprint:
	{
		MovementComp_Cached->OnSprint();
		return BuildHandledResult();
	}

	case EMovementActionIntent::Jump:
	{
		MovementComp_Cached->OnJump();
		return BuildHandledResult();
	}

	default:
		return BuildIgnoredResult();
	}
}

FActionRequestResult UCActionOrchestratorComponent::RequestEquipmentAction(const FEquipmentActionRequest& InActionRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;
	if (!CanAcceptActionRequest(rejectReason)) return BuildRejectedResult(rejectReason);

	if (!IsValid(WeaponComp_Cached) || !IsValid(ActionComp_Cached))
		return BuildRejectedResult(EActionRequestRejectReason::InvalidComponent);

	const EActionType resolvedActionType = ResolveEquipmentActionType(InActionRequest);
	if (resolvedActionType == EActionType::Max) return BuildIgnoredResult();

	const FActionExecutionResult actionExecutionResult = ActionComp_Cached->ExecuteAction(resolvedActionType);
	return BuildRequestResult(actionExecutionResult);
}

FActionRequestResult UCActionOrchestratorComponent::RequestCombatAction(const FCombatActionRequest& InActionRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;
	if (!CanAcceptActionRequest(rejectReason)) return BuildRejectedResult(rejectReason);

	if (!IsValid(ActionComp_Cached)) return BuildRejectedResult(EActionRequestRejectReason::InvalidComponent);

	const EActionType resolvedActionType = ResolveCombatActionType(InActionRequest);
	if (resolvedActionType == EActionType::Max) return BuildIgnoredResult();

	const FActionExecutionResult actionExecutionResult = ActionComp_Cached->ExecuteAction(resolvedActionType);
	return BuildRequestResult(actionExecutionResult);
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

	if (executionState == EExecutionState::Reaction)
	{
		OutRejectReason = EActionRequestRejectReason::InReaction;
		return false;
	}

	if (executionState == EExecutionState::Dead)
	{
		OutRejectReason = EActionRequestRejectReason::Dead;
		return false;
	}

	return true;
}

FActionRequestResult UCActionOrchestratorComponent::BuildRequestResult(const FActionExecutionResult& InActionExecutionResult) const
{
	switch (InActionExecutionResult.Decision)
	{
	case EActionExecutionDecision::Start:		return BuildStartedResult(InActionExecutionResult.ActionType);
	case EActionExecutionDecision::Chain:		return BuildChainedResult(InActionExecutionResult.ActionType);
	case EActionExecutionDecision::Enqueue:		return BuildEnqueuedResult(InActionExecutionResult.ActionType);
	case EActionExecutionDecision::Interrupt:	return BuildInterruptedResult(InActionExecutionResult.ActionType);
	case EActionExecutionDecision::Ignore:		return BuildIgnoredResult();
	case EActionExecutionDecision::Reject:
	default:
		return BuildRejectedResult(EActionRequestRejectReason::NoExecutableAction);
	}
}

FActionRequestResult UCActionOrchestratorComponent::BuildHandledResult(EActionType InResolvedActionType) const
{
	FActionRequestResult result;
	result.ResultType = EActionRequestResultType::Handled;
	result.ResolvedActionType = InResolvedActionType;
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildStartedResult(EActionType InResolvedActionType) const
{
	FActionRequestResult result;
	result.ResultType = EActionRequestResultType::Started;
	result.ResolvedActionType = InResolvedActionType;
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildChainedResult(EActionType InResolvedActionType) const
{
	FActionRequestResult result;
	result.ResultType = EActionRequestResultType::Chained;
	result.ResolvedActionType = InResolvedActionType;
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildEnqueuedResult(EActionType InResolvedActionType) const
{
	FActionRequestResult result;
	result.ResultType = EActionRequestResultType::Enqueued;
	result.ResolvedActionType = InResolvedActionType;
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildInterruptedResult(EActionType InResolvedActionType) const
{
	FActionRequestResult result;
	result.ResultType = EActionRequestResultType::Interrupted;
	result.ResolvedActionType = InResolvedActionType;
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildRejectedResult(EActionRequestRejectReason InRejectReason) const
{
	FActionRequestResult result;
	result.ResultType = EActionRequestResultType::Rejected;
	result.RejectReason = InRejectReason;
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildIgnoredResult() const
{
	FActionRequestResult result;
	result.ResultType = EActionRequestResultType::Ignored;
	return result;
}

EActionType UCActionOrchestratorComponent::ResolveEquipmentActionType(const FEquipmentActionRequest& InActionRequest) const
{
	switch (InActionRequest.IntentType)
	{
	case EEquipmentActionIntent::Equip:		return EActionType::Equip;
	case EEquipmentActionIntent::Unequip:	return EActionType::Unequip;
	case EEquipmentActionIntent::Toggle:
	{
		if (!IsValid(WeaponComp_Cached)) return EActionType::Max;
		return WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed) ? EActionType::Equip : EActionType::Unequip;
	}

	default:
		return EActionType::Max;
	}
}

EActionType UCActionOrchestratorComponent::ResolveCombatActionType(const FCombatActionRequest& InActionRequest) const
{
	switch (InActionRequest.IntentType)
	{
	case ECombatActionIntent::ComboAttack:
		return EActionType::ComboAttack;

	default:
		return EActionType::Max;
	}
}