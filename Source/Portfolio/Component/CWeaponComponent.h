#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CWeaponStructure.h"
#include "CWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponTypeChanged, class ACharacter*, InOwnerCharacter, EWeaponType, InPrevWeaponType, EWeaponType, InNewWeaponType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

	// === WeaponData ======================================= //
private:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	EWeaponType WeaponActorClassKey = EWeaponType::Max;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class ACWeaponActor> WeaponActorClass;

	// ====================================================== //

private:
	/* === State === */
	UPROPERTY(Transient)
	EWeaponType CurrentWeaponType = EWeaponType::Unarmed;

	UPROPERTY(Transient)
	class ACWeaponActor* WeaponActor = nullptr;

private:
	/* === Injected Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalSourceComponent* CombatSignalSourceComp_Injected = nullptr;

public:
	/* === [Out] Custom Delgate Events === */
	FWeaponTypeChanged OnWeaponTypeChanged;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	/* === Check / Query === */
	FORCEINLINE bool CheckCurrentWeaponType(EWeaponType InNewWeaponType) const { return CurrentWeaponType == InNewWeaponType; }

public:
	/* === Getter === */
	FORCEINLINE EWeaponType GetCurrentWeaponType() { return CurrentWeaponType; }

public:
	class ACWeaponActor* GetWeaponActor();

public:
	void AttachWeaponToHand();
	void AttachWeaponToHolster();

public:
	void CommitEquipWeapon();
	void CommitUnequipWeapon();

public:
	void PushContext(const FActionContext& InActionContext);
	void ClearContext();
	void ClearRuntimeWeaponState();

public:
	void OpenCollisionWindow(FName InCollisionName);
	void CloseCollisionWindow();

private:
	void ChangeWeaponType(EWeaponType InNewWeaponType);

private:
	FWeaponContext BuildWeaponContext() const;

private:
	FCharacterComponentReferences BuildWeaponActorReferences() const;
	bool CreateWeaponActor(AActor* InOwnerCharacter, EWeaponType InWeaponType, TSubclassOf<ACWeaponActor> InWeaponActorClass);
};
