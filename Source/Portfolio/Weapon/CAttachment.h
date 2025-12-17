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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	void InitializeAttachment();

public:
	// Bind Delegate
	UFUNCTION()
	void OnEquipmentBeginEquip();

	UFUNCTION()
	void OnEquipmentBeginUnequip();

protected:
	UFUNCTION(BlueprintCallable, Category = "Attachment")
	void AttachToOwnerSocket(FName InSocketName);
};
