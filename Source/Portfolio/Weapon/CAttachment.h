#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAttachment.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttachmentCollisionEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAttachmentCollisionDisabled);

UCLASS()
class PORTFOLIO_API ACAttachment : public AActor
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
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;
	TArray<class UShapeComponent*> Collisions_Cached;

public:
	/* === [OUT] Custom Delgate Events === */
	// Collision
	FAttachmentCollisionEnabled OnAttachmentCollisionEnabled;
	FAttachmentCollisionDisabled OnAttachmentCollisionDisabled;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	void InitializeAttachment();

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
	void Print_BeginOverlapEventInfo(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void Print_EndOverlapEventInfo(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
