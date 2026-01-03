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
	UPROPERTY(Transient)
	FAttachmentContext AttachmentContext;

	UPROPERTY(Transient)
	FEquipmentContext EquipmentContext;
	
	UPROPERTY(Transient)
	FActionContext ActionContext;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;
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
	virtual FAttachmentContext GetAttachmentContext() const override;
	virtual FEquipmentContext GetEquipmentContext() const override;
	virtual FActionContext GetActionContext() const override;

public:
	/* === IHitContextProducer (Setter) === */
	virtual void SetAttachmentContext(FAttachmentContext InAttachmentContext) override;
	virtual void SetEquipmentContext(FEquipmentContext InEquipmentContext) override;
	virtual void SetActionContext(FActionContext InActionContext) override;

public:
	/* === Getter === */
	EAttachmentType GetAttachmentType() const;

public:
	/* === Setter === */
	void SetAttachmentType(EAttachmentType InAttachmentType);

public:
	void AttachToOwnerSocket(FName InSocketName);

public:
	UFUNCTION()
	void OnBeginPlayAction();
	
	UFUNCTION()
	void OnEndPlayAction();
	
	UFUNCTION()
	void OnNextPlayAction();

protected:
	void PushAttachmentContext(const FAttachmentContext& InAttachmentContext);

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
	void Print_BeginOverlapEventInfo(AActor* InAttackerActor, AActor* InDamageCauser, UShapeComponent* InAttackCollision, AActor* InTargetActor, UPrimitiveComponent* InHitComponent, int32 OtherBodyIndex, bool bFromSweep);
	void Print_EndOverlapEventInfo(ACharacter* attacker, AActor* targetActor);
	void Print_ActionContextInfo(FActionContext InActionContext);
};
