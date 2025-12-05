#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

private:
	class ACharacter* OwnerCharacter_Cached;
	class UCMovementComponent* MovementComp_Cached;

public:
	void NativeBeginPlay() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;
};
