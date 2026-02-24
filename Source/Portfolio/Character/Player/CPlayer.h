#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/TargetContextProducer.h"
#include "CPlayer.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayer : public ACharacter, public ITargetContextProducer
{
	GENERATED_BODY()

public:
	ACPlayer();

private:
	UPROPERTY(EditAnywhere, Category = "Priority")
	int Priority = INT_MAX;

private:
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere)
	class UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	class UCWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere)
	class UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere)
	class UCActionComponent* ActionComponent;

	UPROPERTY(VisibleAnywhere)
	class UCApplyDamageComponent* ApplyDamageComponent;

protected:
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UCMovementComponent* GetMovementComp() const;
	UCWeaponComponent* GetWeaponComp() const;
	UCStateComponent* GetStateComp() const;
	UCActionComponent* GetActionComp() const;

public:
	// Interface API
	virtual int GetTargetPriority() const override { return Priority; }

public:
	void HandleMoveForward(const float InAxisValue);
	void HandleMoveRight(const float InAxisValue);

	void HandleWalk();
	void HandleRun();

	void HandleJump();
	void HandleStopJump();

	void HandleComboAction();

	void HandleSword();
};