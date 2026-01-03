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
	/* === Injection Objects === */
	class ACharacter* OwnerCharacter_Injected;
	FEquipmentData EquipmentData_Injected;
	FEquipmentData UnquipmentData_Injected;

private:
	/* === Cached Objects === */
	class UCMovementComponent* MovementComp_Cached;
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

public:
	UFUNCTION()
	void OnBeginPlayAction();

	UFUNCTION()
	void OnEndPlayAction();

	UFUNCTION()
	void OnNextPlayAction();

private:
	void PushEquipmentContext(const FEquipmentContext& InEquipmentContext);
};
