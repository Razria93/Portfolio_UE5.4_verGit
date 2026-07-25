#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CWeaponTypes.h"
#include "Type/CActionKeyTypes.h"
#include "CWeaponComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWeaponTypeChanged, class ACharacter*, InOwnerCharacter, EWeaponType, InPrevWeaponType, EWeaponType, InNewWeaponType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCWeaponComponent();

private:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	EWeaponType WeaponActorClassKey = EWeaponType::Max;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class ACWeaponActor> WeaponActorClass;

private:
	UPROPERTY(Transient)
	EWeaponType CurrentWeaponType = EWeaponType::Unarmed;

	UPROPERTY(Transient)
	class ACWeaponActor* WeaponActor = nullptr;

	UPROPERTY(Transient)
	bool bWeaponActorDisabledForProfiling = false;

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalSourceComponent* CombatSignalSourceComp_Injected = nullptr;

public:
	FWeaponTypeChanged OnWeaponTypeChanged;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Check / Query
	FORCEINLINE bool CheckCurrentWeaponType(EWeaponType InNewWeaponType) const { return CurrentWeaponType == InNewWeaponType; }

public:
	// Query
	FORCEINLINE EWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }

public:
	class ACWeaponActor* GetWeaponActor() const;

public:
	// Mutation
	void AttachWeaponToHand();
	void AttachWeaponToHolster();

public:
	void CommitEquipWeapon();
	void CommitUnequipWeapon();

public:
	void PushActionDataKey(const FActionDataKey& InActionDataKey);
	void ClearContext();
	void ClearWeaponRuntimeState();

public:
	void OpenCollisionWindow(FName InCollisionName);
	void CloseCollisionWindow();

private:
	void ChangeWeaponType(EWeaponType InNewWeaponType);

private:
	// Runtime Lifecycle
	void UninitializeWeaponRuntime();

	// Weapon Actor
	void DestroyWeaponActor();

private:
	FWeaponContext BuildWeaponContext() const;

private:
	FCharacterComponentReferences BuildWeaponActorReferences() const;
	bool CreateWeaponActor(AActor* InOwnerCharacter, EWeaponType InWeaponType, TSubclassOf<ACWeaponActor> InWeaponActorClass);

private:
	// Profiling
	bool ShouldSkipWeaponActorCreationForProfiling() const;
	void SkipWeaponActorCreationForProfiling();
	bool PreserveEquipWeaponTypeWithoutActorForProfiling();
};
