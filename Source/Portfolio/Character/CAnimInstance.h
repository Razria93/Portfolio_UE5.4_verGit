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
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float Speed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	float Direction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Settings")
	bool bIsInAir;

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Type")
	EWeaponType WeaponType = EWeaponType::Max;

private:
	class ACharacter* OwnerCharacter_Cached;
	class UCMovementComponent* MovementComp_Cached;
	class UCWeaponComponent* WeaponComp_Cached;

public:
	void NativeBeginPlay() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	// Bind Delegate
	UFUNCTION()
	void OnWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPrevWeaponType, EWeaponType InNewWeaponType);

};
