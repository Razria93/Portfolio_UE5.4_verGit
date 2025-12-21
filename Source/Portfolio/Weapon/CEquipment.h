#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "CEquipment.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentBeginEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentEndEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentBeginUnequip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentEndUnequip);

UCLASS()
class PORTFOLIO_API UCEquipment : public UObject
{
	GENERATED_BODY()

private:
	bool bBeginEquip;
	bool bBeginUnequip;
	bool bEquipped;

private:
	// Dependency Injection
	class ACharacter* OwnerCharacter_Injected;
	FEquipmentData EquipmentData_Injected;
	FEquipmentData UnquipmentData_Injected;

	// Cached
	class UCMovementComponent* MovementComp_Cached;
	class UCStateComponent* StateComp_Cached;

public:
	// delegate
	FEquipmentBeginEquip OnEquipmentBeginEquip;
	FEquipmentEndEquip OnEquipmentEndEquip;
	FEquipmentBeginUnequip OnEquipmentBeginUnequip;
	FEquipmentEndUnequip OnEquipmentEndUnequip;


public:
	void InitializeEquipment(class ACharacter* InOwnerCharacter, FEquipmentData InEquipmentData, FEquipmentData InUnequipmentData);

public:
	// Trigger: PressKey
	void Equip();
	void Unequip();

	// Trigger: Notify
	void Begin_Equip();
	void End_Equip();
	void Begin_Unequip();
	void End_Unequip();
};
