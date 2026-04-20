#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CWeaponStructure.h"
#include "CWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponTypeChanged, class ACharacter*, InOwnerCharacter, EWeaponType, InPrevWeaponType, EWeaponType, InNewWeaponType);
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
	EWeaponType WeaponType;

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
	EWeaponType CurrentWeaponType_Cached;

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
	FWeaponTypeChanged OnWeaponTypeChanged;
	FEquipmentTypeChanged OnEquipmentTypeChanged;

protected:
	void BeginPlay() override;

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurWeaponType(EWeaponType InNewWeaponType) const { return CurrentWeaponType_Cached == InNewWeaponType; }

public:
	/* === Getter === */
	class UObject* GetWeaponActor();
	class UObject* GetEquipment();

public:
	/* === Getter === */
	FORCEINLINE EWeaponType GetCurWeaponType() { return CurrentWeaponType_Cached; }
	FORCEINLINE EEquipmentType GetCurEquipmentType() { return CurrentEquipmentType_Cached; }

public:
	/* === Setter === */
	void SetUnarmedMode();
	void SetSwordMode();

public:
	void PushContextToWeaponActor(const FActionContext& InActionContext);
	void ClearContextToWeaponActor();

private:
	bool CreateWeaponActor(AActor* InOwnerCharacter, EWeaponType InWeaponType, TSubclassOf<ACWeaponActor> InWeaponActorClass);
	bool CreateEquipment(AActor* InOwnerCharacter, EEquipmentType InEquipmentType, TSubclassOf<UCEquipment> InEquipmentClass, const FEquipmentData& InEquipmentDatas, const FEquipmentData& InUnequipmentDatas);

private:
	void ChangeMode(EWeaponType InNewWeaponType);

private:
	void ChangeWeaponType(EWeaponType InNewWeaponType);
	void ChangeEquipmentType(EEquipmentType InNewEquipmentType);

private:
	FWeaponActorContext BuildWeaponActorContext() const;
	FEquipmentContext BuildEquipmentContext() const;
};
