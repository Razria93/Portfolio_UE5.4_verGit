#include "Weapon/CEquipment.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"

#include "Interface/HitContextProducer.h"

void UCEquipment::InitializeEquipment(ACharacter* InOwnerCharacter, EEquipmentType InEquipmentType, FEquipmentData InEquipmentData, FEquipmentData InUnequipmentData)
{
	SetEquipmentType(InEquipmentType);

	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	EquipmentData_Injected = InEquipmentData;
	UnquipmentData_Injected = InUnequipmentData;

	bBeginEquip = false;
	bBeginUnequip = false;
	bEquipped = false;

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Injected->GetComponentByClass(UCMovementComponent::StaticClass())); // TODO: Refactor Interface
	check(MovementComp_Cached);

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Injected->GetComponentByClass(UCStateComponent::StaticClass())); // TODO: Refactor Interface
	check(StateComp_Cached);
}

EEquipmentType UCEquipment::GetEquipmentType() const
{
	return EquipmentType;
}

void UCEquipment::SetEquipmentType(EEquipmentType InEquipmentType)
{
	EquipmentType = InEquipmentType;
}

void UCEquipment::Equip()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached))
		return;

	StateComp_Cached->SetEquipMode();
	
	if (EquipmentData_Injected.bCanMove == false)
		MovementComp_Cached->SetStop();
	
	if (IsValid(EquipmentData_Injected.Montage))
	{
		OwnerCharacter_Injected->PlayAnimMontage(EquipmentData_Injected.Montage, EquipmentData_Injected.PlayRate);
		return;
	}
}

void UCEquipment::Unequip()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached))
		return;

	StateComp_Cached->SetUnequipMode();

	if (UnquipmentData_Injected.bCanMove == false)
		MovementComp_Cached->SetStop();

	if (IsValid(UnquipmentData_Injected.Montage))
	{
		OwnerCharacter_Injected->PlayAnimMontage(UnquipmentData_Injected.Montage, UnquipmentData_Injected.PlayRate);
		return;
	}
}

void UCEquipment::Begin_Equip()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached)) return;

	bBeginEquip = true;

	if (OnEquipmentBeginEquip.IsBound())
		OnEquipmentBeginEquip.Broadcast();
}

void UCEquipment::End_Equip()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached)) return;

	bBeginEquip = false;
	bEquipped = true;

	if (EquipmentData_Injected.bCanMove == false)
		MovementComp_Cached->SetMove();

	if (OnEquipmentEndEquip.IsBound())
		OnEquipmentEndEquip.Broadcast();

	StateComp_Cached->SetIdleMode();
}

void UCEquipment::Begin_Unequip()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached)) return;

	bBeginUnequip = true;

	if (OnEquipmentBeginUnequip.IsBound())
		OnEquipmentBeginUnequip.Broadcast();
}

void UCEquipment::End_Unequip()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(MovementComp_Cached) || !IsValid(StateComp_Cached)) return;

	bBeginUnequip = false;
	bEquipped = false;

	if (UnquipmentData_Injected.bCanMove == false)
		MovementComp_Cached->SetMove();

	if (OnEquipmentEndUnequip.IsBound())
		OnEquipmentEndUnequip.Broadcast();

	StateComp_Cached->SetIdleMode();
}

void UCEquipment::OnBeginPlayAction()
{
	FEquipmentContext equipmentContext;
	equipmentContext.CurrentEquipmentType = EquipmentType;

	PushEquipmentContext(equipmentContext);
}

void UCEquipment::OnEndPlayAction()
{
	FEquipmentContext equipmentContext = FEquipmentContext();

	PushEquipmentContext(equipmentContext);
}

void UCEquipment::OnNextPlayAction()
{
	FEquipmentContext equipmentContext;
	equipmentContext.CurrentEquipmentType = EquipmentType;

	PushEquipmentContext(equipmentContext);
}

void UCEquipment::PushEquipmentContext(const FEquipmentContext& InEquipmentContext)
{
	UObject* uobject = WeaponComp_Cached->GetAttachment();
	if (!IsValid(uobject)) return;

	IHitContextProducer* producer = Cast<IHitContextProducer>(uobject);
	if (!producer) return;

	producer->SetEquipmentContext(InEquipmentContext);
}