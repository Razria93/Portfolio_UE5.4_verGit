#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPlayer.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	ACPlayer();

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UCMovementComponent* MovementComp;

protected:
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void HandleMoveForward(const float InAxisValue);
	void HandleMoveRight(const float InAxisValue);
	void HandleWalk();
	void HandleRun();
};