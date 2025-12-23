#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAttachment.generated.h"

UCLASS()
class PORTFOLIO_API ACAttachment : public AActor
{
	GENERATED_BODY()

public:
	ACAttachment();

public:
	UPROPERTY(EditAnywhere)
	FName SocketName_Holster;	// Initialize on Editor

	UPROPERTY(EditAnywhere)
	FName SocketName_Hand;		// Initialize on Editor

protected:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* Root;

private:
	// Cached
	class ACharacter* OwnerCharacter_Cached;
	TArray<class UShapeComponent*> Collisions_Cached;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	void InitializeAttachment();

public:
	// Called when 'Equipment' broadcasts 'OnEquipmentBeginEquip()' event
	UFUNCTION()
	void OnEquipmentBeginEquip();

	// Called when 'Equipment' broadcasts 'OnEquipmentBeginUnequip()' event
	UFUNCTION()
	void OnEquipmentBeginUnequip();

public:
	// Called when 'UShapeComponent' broadcasts 'OnComponentBeginOverlap()' event
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Called when 'UShapeComponent' broadcasts 'OnComponentEndOverlap()' event
	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	void OnCollision(FName InName);
	void OffCollision();

protected:
	UFUNCTION(BlueprintCallable, Category = "Attachment")
	void AttachToOwnerSocket(FName InSocketName);

private:
	void Print_BeginOverlapEventInfo(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void Print_EndOverlapEventInfo(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
