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

public:
	/* === Editor Settings === */
	UPROPERTY(EditAnywhere, Category = "Attach|SocketName")
	FName SocketName_Holster;

	UPROPERTY(EditAnywhere, Category = "Attach|SocketName")
	FName SocketName_Hand;

private:
	UPROPERTY(EditAnywhere, Category = "Feedback|Trail")
	bool bDisableTrailOnBeginPlay = true;

protected:
	/* === Components === */
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* RootSceneComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* TrailComponent = nullptr;

private:
	UPROPERTY(Transient)
	EWeaponType WeaponType;

public:
	/* === Context Carrier === */
	UPROPERTY(Transient)
	FOverlapContext LastOverlapContext;

	UPROPERTY(Transient)
	FWeaponActorContext LastWeaponActorContext;

	UPROPERTY(Transient)
	FEquipmentContext LastEquipmentContext;

	UPROPERTY(Transient)
	FActionContext LastActionContext;

private:
	UPROPERTY(Transient)
	bool bHitWindowOpened = false;

	UPROPERTY(Transient)
	int32 CurrentHitWindowId = INDEX_NONE;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;
	class UCApplyDamageComponent* ApplyDamageComp_Cached;
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
	virtual void Tick(float DeltaTime) override;

public:
	void InitializeWeaponActor(EWeaponType InWeaponType);

public:
	/* === IHitContextProducer (Getter) === */
	virtual const FOverlapContext& GetLastOverlapContext() const override;
	virtual const FWeaponActorContext& GetLastWeaponActorContext() const override;
	virtual const FEquipmentContext& GetLastEquipmentContext() const override;
	virtual const FActionContext& GetLastActionContext() const override;

public:
	/* === IHitContextProducer (Setter) === */
	virtual void SetLastOverlapContext(const FOverlapContext& InOverlapContext) override;
	virtual void SetLastWeaponActorContext(const FWeaponActorContext& InWeaponActorContext) override;
	virtual void SetLastEquipmentContext(const FEquipmentContext& InEquipmentContext) override;
	virtual void SetLastActionContext(const FActionContext& InActionContext) override;

public:
	/* === Getter === */
	EWeaponType GetWeaponType() const;

public:
	/* === Setter === */
	void SetWeaponType(EWeaponType InWeaponType);
	void SetTrailActive(bool bEnable);

public:
	void AttachToOwnerSocket(FName InSocketName);

public:
	/* === [IN] Engine Delgate Events === */
	// UShapeComponent
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	/* === [IN] Custom Delgate Events === */
	// CEquipment
	UFUNCTION()
	void OnEquipmentBeginEquip();

	UFUNCTION()
	void OnEquipmentBeginUnequip();

public:
	/* === AnimNotify Events === */
	// CAnimNotify_Collision
	void CollisionEnabled(FName InName);
	void CollisionDisabled();

private:
	FOverlapContext BuildOverlapContext(AActor* InOwnerActor, AActor* InDamageCauser, UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) const;
	FHitContext BuildHitContext(const FOverlapContext& InOverlapContext) const;

private:
	void PrintBeginOverlapContextInfo(const FHitContext& InHitContext);
	void PrintEndOverlapContextInfo(const FHitContext& InHitContext);

private:
	void PrintOverlapContextInfo(const FOverlapContext& Context);
	void PrintHitContextInfo(const FWeaponActorContext& InWeaponActorContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext);

private:
	void PrintTrailInfo(bool bEnable) const;
};
