#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/HitContextProvider.h"
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

	// === Weapon Actor Data ================================ //
public:
	UPROPERTY(EditAnywhere, Category = "Weapon|SocketName")
	FName SocketName_Holster;

	UPROPERTY(EditAnywhere, Category = "Weapon|SocketName")
	FName SocketName_Hand;

private:
	UPROPERTY(EditAnywhere, Category = "Feedback|Trail")
	bool bDisableTrailOnBeginPlay = true;

	// ====================================================== //

private:
	UPROPERTY(Transient)
	EWeaponType WeaponType;

private:
	UPROPERTY(Transient)
	bool bHitWindowOpened = false;

	UPROPERTY(Transient)
	int32 CurrentHitWindowId = INDEX_NONE;

private:
	/* === Components === */
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* RootSceneComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* TrailComponent = nullptr;

private:
	/* === Context Carrier === */
	UPROPERTY(Transient)
	FOverlapContext LastOverlapContext_Cached;

	UPROPERTY(Transient)
	FWeaponContext LastWeaponContext_Cached;

	UPROPERTY(Transient)
	FActionContext LastActionContext_Cached;


private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

	UPROPERTY(Transient)
	class UCApplyDamageComponent* ApplyDamageComp_Cached;

private:
	UPROPERTY(Transient)
	TArray<class UShapeComponent*> Collisions_Cached;

public:
	/* === [OUT] Custom Delgate Events === */
	// Collision (Enabled/Disabled)
	FWeaponActorCollisionEnabled OnWeaponActorCollisionEnabled;
	FWeaponActorCollisionDisabled OnWeaponActorCollisionDisabled;

	// Overlap (Raw Overlap)
	FWeaponActorBeginOverlap OnWeaponActorBeginOverlap;
	FWeaponActorEndOverlap OnWeaponActorEndOverlap;

protected:
	virtual void BeginPlay() override;

public:
	void InitializeWeaponActor(EWeaponType InWeaponType);

public:
	/* === IHitContextProducer (Getter) === */
	virtual const FOverlapContext& GetLastOverlapContext() const override;
	virtual const FWeaponContext& GetLastWeaponContext() const override;
	virtual const FActionContext& GetLastActionContext() const override;

public:
	/* === IHitContextProducer (Setter) === */
	virtual void SetLastOverlapContext(const FOverlapContext& InOverlapContext) override;
	virtual void SetLastWeaponContext(const FWeaponContext& InWeaponContext) override;
	virtual void SetLastActionContext(const FActionContext& InActionContext) override;

public:
	/* === Getter === */
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }

public:
	FORCEINLINE bool IsHitWindowOpened() const { return bHitWindowOpened; }
	FORCEINLINE int32 GetCurrentHitWindowId() const { return CurrentHitWindowId; }

public:
	/* === Setter === */
	void ChangeWeaponType(EWeaponType InWeaponType);
	void ToggleTrailActive(bool bEnable);

public:
	/* === AnimNotify Events === */
	// CAnimNotify_Equip / Unequip
	void AttachToHandSocket();
	void AttachToHolsterSocket();

public:
	/* === AnimNotify Events === */
	// CAnimNotify_Collision
	void CollisionEnabled(FName InName);
	void CollisionDisabled();

public:
	/* === [IN] Engine Delgate Events === */
	// UShapeComponent
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

private:
	void PrintBeginOverlapContextInfo(const FHitContext& InHitContext);
	void PrintEndOverlapContextInfo(const FHitContext& InHitContext);

private:
	void PrintOverlapContextInfo(const FOverlapContext& Context);
	void PrintHitContextInfo(const FWeaponContext& InWeaponContext, const FActionContext& InActionContext);

private:
	void PrintTrailInfo(bool bEnable) const;
};
