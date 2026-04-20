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
	PrimaryComponentTick.bCanEverTick = true;
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

void UCActionOrchestratorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FActionRequestResult UCActionOrchestratorComponent::RequestMovementAction(const FMovementActionRequest& InRequest)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached))
		return BuildRejectedResult(EActionRequestRejectReason::InvalidComponent);

	switch (InRequest.IntentType)
	{
	case EMovementActionIntent::StopJump:
		MovementComp_Cached->OnStopJump();
		return BuildExecutedResult();

	default:
		break;
	}

	// Out Parameter
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;
	if (!CanAcceptActionRequest(rejectReason)) return BuildRejectedResult(rejectReason);

	// TODO
	switch (InRequest.IntentType)
	{
	case EMovementActionIntent::Walk:
		MovementComp_Cached->OnWalk();
		return BuildExecutedResult();

	case EMovementActionIntent::Run:
		MovementComp_Cached->OnRun();
		return BuildExecutedResult();

	case EMovementActionIntent::Jump:
		MovementComp_Cached->OnJump();
		return BuildExecutedResult();

	default:
		return BuildIgnoredResult();
	}
}

FActionRequestResult UCActionOrchestratorComponent::RequestEquipmentAction(const FEquipmentActionRequest& InRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;
	if (!CanAcceptActionRequest(rejectReason)) return BuildRejectedResult(rejectReason);

	if (!IsValid(StateComp_Cached) || !IsValid(WeaponComp_Cached))
		return BuildRejectedResult(EActionRequestRejectReason::InvalidComponent);

	if (!StateComp_Cached->CheckCurExecutionState(EExecutionState::Idle))
		return BuildRejectedResult(EActionRequestRejectReason::InvalidState);

	// TODO
	switch (InRequest.IntentType)
	{
	case EEquipmentActionIntent::Toggle:
		if (WeaponComp_Cached->CheckCurAttachmentType(EAttachmentType::Unarmed))
		{
			WeaponComp_Cached->SetSwordMode();
			return BuildExecutedResult();
		}

		if (WeaponComp_Cached->CheckCurAttachmentType(EAttachmentType::Sword))
		{
			WeaponComp_Cached->SetUnarmedMode();
			return BuildExecutedResult();
		}

		return BuildRejectedResult(EActionRequestRejectReason::InvalidEquipment);

	default:
		return BuildIgnoredResult();
	}
}

FActionRequestResult UCActionOrchestratorComponent::RequestCombatAction(const FCombatActionRequest& InRequest)
{
	EActionRequestRejectReason rejectReason = EActionRequestRejectReason::None;
	if (!CanAcceptActionRequest(rejectReason)) return BuildRejectedResult(rejectReason);

	if (!IsValid(ActionComp_Cached))
		return BuildRejectedResult(EActionRequestRejectReason::InvalidComponent);

	// TODO
	switch (InRequest.IntentType)
	{
	case ECombatActionIntent::ComboAttack:
		ActionComp_Cached->SetComboAttackMode();
		return BuildExecutedResult(EActionType::ComboAttack);

	default:
		return BuildIgnoredResult();
	}
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

	const EExecutionState executionState = StateComp_Cached->GetCurExecutionState();

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

FActionRequestResult UCActionOrchestratorComponent::BuildExecutedResult(EActionType InExecutedActionType) const
{
	FActionRequestResult result;

	result.ResultType = EActionRequestResultType::Executed;
	result.ExecutedActionType = InExecutedActionType; // Execute Factor
	
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildRejectedResult(EActionRequestRejectReason InRejectReason) const
{
	FActionRequestResult result;

	result.ResultType = EActionRequestResultType::Rejected;
	result.RejectReason = InRejectReason; // Reject Factor
	
	return result;
}

FActionRequestResult UCActionOrchestratorComponent::BuildIgnoredResult() const
{
	FActionRequestResult result;

	result.ResultType = EActionRequestResultType::Ignored;
	
	return result;
}

