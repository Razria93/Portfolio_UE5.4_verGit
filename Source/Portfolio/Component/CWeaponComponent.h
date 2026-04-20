#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponActorTypeChanged, class ACharacter*, InOwnerCharacter, EWeaponActorType, InPrevWeaponActorType, EWeaponActorType, InNewWeaponActorType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEquipmentTypeChanged, class ACharacter*, InOwnerCharacter, EEquipmentType, InPrevEquipmentType, EEquipmentType, InNewEquipmentType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

	// === WeaponData ======================================= //
private:
	UPROPERTY(EditAnywhere, Category = "WeaponActor|Data")
	EWeaponActorType WeaponActorType;

	UPROPERTY(EditAnywhere, Category = "Equipment|Data")
	EEquipmentType EquipmentType;

private:
	UPROPERTY(EditAnywhere, Category = "WeaponActor|Data")
	TSubclassOf<class ACWeaponActor> WeaponActorClass;

	UPROPERTY(EditAnywhere, Category = "Equipment|Data")
	TSubclassOf<class UCEquipment> EquipmentClass;

private:
	UPROPERTY(EditAnywhere, Category = "Equipment|Data")
	FEquipmentData EquipmentData;

	UPROPERTY(EditAnywhere, Category = "Equipment|Data")
	FEquipmentData UnequipmentData;

	// ====================================================== //

private:
	/* === State === */
	UPROPERTY(Transient)
	EWeaponActorType CurrentWeaponActorType_Cached;

	UPROPERTY(Transient)
	EEquipmentType CurrentEquipmentType_Cached;

private:
	UPROPERTY(Transient)
	class ACWeaponActor* WeaponActor;

	UPROPERTY(Transient)
	class UCEquipment* Equipment;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

public:
	/* === [Out] Custom Delgate Events === */
	FWeaponActorTypeChanged OnWeaponActorTypeChanged;
	FEquipmentTypeChanged OnEquipmentTypeChanged;

protected:
	void BeginPlay() override;

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurWeaponActorType(EWeaponActorType InNewWeaponActorType) const { return CurrentWeaponActorType_Cached == InNewWeaponActorType; }

public:
	/* === Getter === */
	class UObject* GetWeaponActor();
	class UObject* GetEquipment();

public:
	/* === Getter === */
	FORCEINLINE EWeaponActorType GetCurWeaponActorType() { return CurrentWeaponActorType_Cached; }
	FORCEINLINE EEquipmentType GetCurEquipmentType() { return CurrentEquipmentType_Cached; }

public:
	/* === Setter === */
	void SetUnarmedMode();
	void SetSwordMode();

public:
	void PushContextToWeaponActor(const FActionContext& InActionContext);
	void ClearContextToWeaponActor();

private:
	bool CreateWeaponActor(AActor* InOwnerCharacter, EWeaponActorType InWeaponActorType, TSubclassOf<ACWeaponActor> InWeaponActorClass);
	bool CreateEquipment(AActor* InOwnerCharacter, EEquipmentType InEquipmentType, TSubclassOf<UCEquipment> InEquipmentClass, const FEquipmentData& InEquipmentDatas, const FEquipmentData& InUnequipmentDatas);

private:
	void ChangeMode(EWeaponActorType InNewWeaponActorType);

private:
	void ChangeWeaponActorType(EWeaponActorType InNewWeaponActorType);
	void ChangeEquipmentType(EEquipmentType InNewEquipmentType);

private:
	FWeaponActorContext BuildWeaponActorContext() const;
	FEquipmentContext BuildEquipmentContext() const;
};
