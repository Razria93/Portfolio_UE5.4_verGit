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
	UPROPERTY(Transient)
	EEquipmentType EquipmentType;

private:
	UPROPERTY(Transient)
	FEquipmentData EquipmentData_Injected;

	UPROPERTY(Transient)
	FEquipmentData UnquipmentData_Injected;

private:
	/* === Injection Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached;

public:
	/* === [OUT] Custom Delgate Events === */
	// BeginEquip
	FEquipmentBeginEquip OnEquipmentBeginEquip;
	FEquipmentEndEquip OnEquipmentEndEquip;

	// EndEquip
	FEquipmentBeginUnequip OnEquipmentBeginUnequip;
	FEquipmentEndUnequip OnEquipmentEndUnequip;


public:
	void InitializeEquipment(class ACharacter* InOwnerCharacter, EEquipmentType InEquipmentType, FEquipmentData InEquipmentData, FEquipmentData InUnequipmentData);

public:
	/* === Getter === */
	EEquipmentType GetEquipmentType() const;

public:
	/* === Setter === */
	void SetEquipmentType(EEquipmentType InEquipmentType);

public:
	void Equip();
	void Unequip();

public:
	/* === AnimNotify Events === */
	// UCAnimNotify_Unequip
	void Begin_Equip();
	void End_Equip();
	void Begin_Unequip();
	void End_Unequip();
};
