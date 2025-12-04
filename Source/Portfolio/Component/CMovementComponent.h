#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CMovementComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMovementComponent();

private:
	class ACharacter* OwnerCharacter_Cached;

private:
	bool bCanMove = true;

protected:
	virtual void BeginPlay() override;

public:
	void OnMoveForward(float InValue);
	void OnMoveRight(float InValue);


};
