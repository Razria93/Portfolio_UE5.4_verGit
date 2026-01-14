#include "Component/CStateComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Type/CStateStructure.h"

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

void UCStateComponent::SetIdleMode()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateMode(EStateType::Idle);
}

void UCStateComponent::SetEquipMode()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateMode(EStateType::Equip);
}

void UCStateComponent::SetUnequipMode()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateMode(EStateType::Unequip);
}

void UCStateComponent::SetActionMode()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateMode(EStateType::Action);
}
 
void UCStateComponent::SetReactionMode()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateMode(EStateType::Reaction);
}

void UCStateComponent::ChangeStateType(EStateType InNewStateType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EStateType prevStateType = CurrentStateType;
	
	CurrentStateType = InNewStateType;

	if (OnStateTypeChanged.IsBound())
		OnStateTypeChanged.Broadcast(OwnerCharacter_Cached, prevStateType, CurrentStateType);
}

void UCStateComponent::ChangeStateMode(EStateType InNewStateType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeStateType(InNewStateType);
}