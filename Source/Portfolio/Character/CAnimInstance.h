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
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EWeaponType CurrentWeaponType = EWeaponType::Max;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EDeadState DeadState = EDeadState::Alive;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached;

public:
	void NativeInitializeAnimation() override;
	void NativeUninitializeAnimation() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	/* === [IN] Custom Delgate Events === */
	// CWeaponComponent
	UFUNCTION()
	void OnWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPrevWeaponType, EWeaponType InNewWeaponType);
};
