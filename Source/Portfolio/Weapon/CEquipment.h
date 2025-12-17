#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Type/CWeaponStructure.h"
#include "CEquipment.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentBeginEquip);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEquipmentEndEquip);

UCLASS()
class PORTFOLIO_API UCEquipment : public UObject
{
	GENERATED_BODY()

private:
	bool bBeginEquip;
	bool bEquipped;

private:
	// Cached
	class ACharacter* OwnerCharacter_Cached;
	class UCMovementComponent* MovementComp_Cached;
	class UCStateComponent* StateComp_Cached;
	FEquipmentData EquipmentData_Cached;

public:
	// delegate
	FEquipmentBeginEquip OnEquipmentBeginEquip;
	FEquipmentEndEquip OnEquipmentEndEquip;

public:
	void InitializeEquipment(class ACharacter* InOwnerCharacter, const FEquipmentData& InEquipmentData);

public:
	// Trigger: PressKey
	void Equip();

	// Trigger: Notify
	void Begin_Equip();
	void End_Equip();
};
