#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/HitContextProducer.h"
#include "Type/CWeaponStructure.h"
#include "CAttachment.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttachmentCollisionEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttachmentCollisionDisabled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_EightParams(FAttachmentBeginOverlap, AActor*, InAttackerActor, AActor*, InDamageCauser, UShapeComponent*, InAttackCollision, AActor*, InTargetActor, UPrimitiveComponent*, InHitComponent, int32, InOtherBodyIndex, bool, InbFromSweep, const FHitResult&, InSweepResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttachmentEndOverlap, AActor*, InAttackerActor, AActor*, InTargetActor);

UCLASS()
class PORTFOLIO_API ACAttachment : public AActor, public IHitContextProducer
{
	GENERATED_BODY()

public:
	ACAttachment();

public:
	/* === Editor Settings === */
	UPROPERTY(EditAnywhere)
	FName SocketName_Holster;

	UPROPERTY(EditAnywhere)
	FName SocketName_Hand;

protected:
	/* === Components === */
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* Root;

private:
	UPROPERTY(Transient)
	EAttachmentType AttachmentType;

public:
	/* === Context Carrier === */
	UPROPERTY(Transient)
	FOverlapContext LastOverlapContext;

	UPROPERTY(Transient)
	FAttachmentContext LastAttachmentContext;

	UPROPERTY(Transient)
	FEquipmentContext LastEquipmentContext;

	UPROPERTY(Transient)
	FActionContext LastActionContext;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;
	class UCApplyDamageComponent* ApplyDamageComp_Cached;
	TArray<class UShapeComponent*> Collisions_Cached;

public:
	/* === [OUT] Custom Delgate Events === */
	// Collision (Enabled/Disabled)
	FAttachmentCollisionEnabled OnAttachmentCollisionEnabled;
	FAttachmentCollisionDisabled OnAttachmentCollisionDisabled;

	// Overlap (Raw Overlap)
	FAttachmentBeginOverlap OnAttachmentBeginOverlap;
	FAttachmentEndOverlap OnAttachmentEndOverlap;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	void InitializeAttachment(EAttachmentType InAttachmentType);

public:
	/* === IHitContextProducer (Getter) === */
	virtual const FOverlapContext& GetLastOverlapContext() const override;
	virtual const FAttachmentContext& GetLastAttachmentContext() const override;
	virtual const FEquipmentContext& GetLastEquipmentContext() const override;
	virtual const FActionContext& GetLastActionContext() const override;

public:
	/* === IHitContextProducer (Setter) === */
	virtual void SetLastOverlapContext(const FOverlapContext& InOverlapContext) override;
	virtual void SetLastAttachmentContext(const FAttachmentContext& InAttachmentContext) override;
	virtual void SetLastEquipmentContext(const FEquipmentContext& InEquipmentContext) override;
	virtual void SetLastActionContext(const FActionContext& InActionContext) override;

public:
	/* === Getter === */
	EAttachmentType GetAttachmentType() const;

public:
	/* === Setter === */
	void SetAttachmentType(EAttachmentType InAttachmentType);

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

private:
	void PrintBeginOverlapContextInfo(const FHitContext& InHitContext);
	void PrintEndOverlapContextInfo(const FHitContext& InHitContext);

	void PrintOverlapContextInfo(const FOverlapContext& Context);
	void PrintHitContextInfo(const FAttachmentContext& InAttachmentContext, const FEquipmentContext& InEquipmentContext, const FActionContext& InActionContext);
};
