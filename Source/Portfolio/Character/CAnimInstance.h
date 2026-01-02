#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Type/CweaponStructure.h"
#include "CAnimInstance.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	/* === Injection Datas === */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float Speed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float Direction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	bool bIsInAir;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Type")
	EAttachmentType AttachmentType = EAttachmentType::Max;

private:
	/* === Cached Objects === */
	class ACharacter* OwnerCharacter_Cached;
	class UCMovementComponent* MovementComp_Cached;
	class UCWeaponComponent* WeaponComp_Cached;

public:
	void NativeBeginPlay() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	/* === [IN] Custom Delgate Events === */
	// CWeaponComponent
	UFUNCTION()
	void OnAttachmentTypeChanged(ACharacter* InOwnerCharacter, EAttachmentType InPrevAttachmentType, EAttachmentType InNewAttachmentType);

};
