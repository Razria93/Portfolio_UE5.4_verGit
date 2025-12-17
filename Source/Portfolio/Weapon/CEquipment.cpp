#include "Weapon/CEquipment.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"

void UCEquipment::InitializeEquipment(ACharacter* InOwnerCharacter, FEquipmentData InEquipmentData)
{
	EquipmentData_Cached = InEquipmentData;
	OwnerCharacter_Cached = InOwnerCharacter;
	check(OwnerCharacter_Cached);

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Cached->GetComponentByClass(UCMovementComponent::StaticClass())); // TODO: Refactor Interface
	check(MovementComp_Cached);

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Cached->GetComponentByClass(UCStateComponent::StaticClass())); // TODO: Refactor Interface
	check(StateComp_Cached);
}

void UCEquipment::Equip()
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached))
		return;

	StateComp_Cached->SetEquipMode();
	
	if (EquipmentData_Cached.bCanMove == false)
		MovementComp_Cached->SetStop();
	
	if (IsValid(EquipmentData_Cached.Montage))
	{
		OwnerCharacter_Cached->PlayAnimMontage(EquipmentData_Cached.Montage, EquipmentData_Cached.PlayRate);
		return;
	}
}

void UCEquipment::Begin_Equip()
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached))
		return;

	bBeginEquip = true;

	if (OnEquipmentBeginEquip.IsBound())
		OnEquipmentBeginEquip.Broadcast();
}

void UCEquipment::End_Equip()
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached))
		return;

	bBeginEquip = false;
	bEquipped = true;

	if (EquipmentData_Cached.bCanMove == false)
		MovementComp_Cached->SetMove();

	if (OnEquipmentEndEquip.IsBound())
		OnEquipmentEndEquip.Broadcast();

	StateComp_Cached->SetIdleMode();
}
