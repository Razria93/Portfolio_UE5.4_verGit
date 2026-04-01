#include "Component/CStateComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Type/CStateStructure.h"
#include "Type/CHealthStructure.h"

UCStateComponent::UCStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentStateType = EStateType::Idle;
}

void UCStateComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);
}

void UCStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCStateComponent::OnDeadStateChanged(EDeadState InPrevDeadState, EDeadState InNewDeadState)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	switch (InNewDeadState)
	{
	case EDeadState::Alive:
		if (CheckCurStateType(EStateType::Dead))
		{
			SetIdleState();
		}
		break;

	case EDeadState::Dying:
	case EDeadState::Dead:
	case EDeadState::Reviving:
		SetDeadState();
		break;

	default:
		break;
	}
}

void UCStateComponent::SetIdleState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateType(EStateType::Idle);
}

void UCStateComponent::SetEquipState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateType(EStateType::Equip);
}

void UCStateComponent::SetUnequipState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateType(EStateType::Unequip);
}

void UCStateComponent::SetActionState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateType(EStateType::Action);
}

void UCStateComponent::SetReactionState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateType(EStateType::Reaction);
}

void UCStateComponent::SetDeadState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateType(EStateType::Dead);
}

void UCStateComponent::ChangeStateType(EStateType InNewStateType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;
	if (CurrentStateType == InNewStateType) return;

	EStateType prevStateType = CurrentStateType;
	CurrentStateType = InNewStateType;

	PrintStateChangedInfo(prevStateType, CurrentStateType);

	if (OnStateTypeChanged.IsBound())
	{
		OnStateTypeChanged.Broadcast(OwnerCharacter_Cached, prevStateType, CurrentStateType);
	}
}

void UCStateComponent::PrintStateChangedInfo(EStateType InPrevStateType, EStateType InNewStateType) const
{
	FLog::Log(FString::Printf(
		TEXT("[StateChanged] Owner = %s | PrevState = %s | NewState = %s"),
		*GetNameSafe(OwnerCharacter_Cached),
		*UEnum::GetValueAsString(InPrevStateType),
		*UEnum::GetValueAsString(InNewStateType)
	));
}