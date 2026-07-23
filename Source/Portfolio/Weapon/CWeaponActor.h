#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/HitContextProvider.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CWeaponStructure.h"
#include "CWeaponActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponActorCollisionEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWeaponActorCollisionDisabled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams(FWeaponActorBeginOverlap, AActor*, InAttackerActor, AActor*, InDamageCauser, UShapeComponent*, InAttackCollision, AActor*, InTargetActor, UPrimitiveComponent*, InHitComponent, int32, InOtherBodyIndex, bool, InbFromSweep, const FHitResult&, InSweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponActorEndOverlap, AActor*, InAttackerActor, AActor*, InTargetActor);

UCLASS()
class PORTFOLIO_API ACWeaponActor : public AActor, public IHitContextProvider
{
	GENERATED_BODY()

public:
	ACWeaponActor();

public:
	UPROPERTY(EditAnywhere, Category = "Weapon|SocketName")
	FName SocketName_Holster;

	UPROPERTY(EditAnywhere, Category = "Weapon|SocketName")
	FName SocketName_Hand;

private:
	UPROPERTY(EditAnywhere, Category = "Feedback|Trail")
	bool bDisableTrailOnBeginPlay = true;

private:
	UPROPERTY(Transient)
	EWeaponType WeaponType;

private:
	UPROPERTY(Transient)
	bool bHitWindowOpened = false;

	UPROPERTY(Transient)
	int32 CurrentHitWindowId = INDEX_NONE;

private:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* RootSceneComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* TrailComponent = nullptr;

private:
	UPROPERTY(Transient)
	FOverlapContext LastOverlapContext_Cached;

	UPROPERTY(Transient)
	FWeaponContext LastWeaponContext_Cached;

	UPROPERTY(Transient)
	FActionContext LastActionContext_Cached;


private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCCombatSignalSourceComponent* CombatSignalSourceComp_Injected = nullptr;

private:
	UPROPERTY(Transient)
	TArray<class UShapeComponent*> Collisions_Cached;

public:
	FWeaponActorCollisionEnabled OnWeaponActorCollisionEnabled;
	FWeaponActorCollisionDisabled OnWeaponActorCollisionDisabled;

	FWeaponActorBeginOverlap OnWeaponActorBeginOverlap;
	FWeaponActorEndOverlap OnWeaponActorEndOverlap;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredReferences() const;

public:
	// Initial State
	void ApplyInitialWeaponState(EWeaponType InWeaponType);

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Collision Component
	void InitializeCollisionComponents();
	void ClearCollisionComponents();

private:
	// Trail
	void InitializeTrailState();
	void ClearTrailState();

public:
	// Hit Context Provider Query
	virtual const FOverlapContext& GetLastOverlapContext() const override;
	virtual const FWeaponContext& GetLastWeaponContext() const override;
	virtual const FActionContext& GetLastActionContext() const override;

public:
	// Hit Context Provider Mutation
	virtual void SetLastOverlapContext(const FOverlapContext& InOverlapContext) override;
	virtual void SetLastWeaponContext(const FWeaponContext& InWeaponContext) override;
	virtual void SetLastActionContext(const FActionContext& InActionContext) override;

public:
	// Query
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

public:
	FORCEINLINE bool IsHitWindowOpened() const { return bHitWindowOpened; }
	FORCEINLINE int32 GetCurrentHitWindowId() const { return CurrentHitWindowId; }

public:
	// Mutation
	void ChangeWeaponType(EWeaponType InWeaponType);
	void ToggleTrailActive(bool bEnable);

public:
	// Equip Notify Events
	void AttachToHandSocket();
	void AttachToHolsterSocket();

public:
	// Collision Notify Events
	void CollisionEnabled(FName InName);
	void CollisionDisabled();

public:
	// Engine Delegate Events
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	FOverlapContext BuildOverlapContext(AActor* InOwnerActor, AActor* InDamageCauser, UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) const;
	FDamageImpactInfo BuildDamageImpactInfo(const FOverlapContext& InOverlapContext) const;
	FHitContext BuildHitContext(const FOverlapContext& InOverlapContext) const;

private:
	void AttachToOwnerSocket(FName InSocketName);
};
