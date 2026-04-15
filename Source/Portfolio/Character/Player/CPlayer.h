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

	UPROPERTY(VisibleAnywhere)
	class UCTakeDamageComponent* TakeDamageComponent;

	UPROPERTY(VisibleAnywhere)
	class UCHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere)
	class UCReactionComponent* ReactionComponent;

	UPROPERTY(VisibleAnywhere)
	class UCActionFeedbackComponent* ActionFeedbackComponent;

	UPROPERTY(VisibleAnywhere)
	class UCReactionFeedbackComponent* ReactionFeedbackComponent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE UCMovementComponent* GetMovementComp() const { return MovementComponent; }
	FORCEINLINE UCWeaponComponent* GetWeaponComp() const { return WeaponComponent; }
	FORCEINLINE UCStateComponent* GetStateComp() const { return StateComponent; }
	FORCEINLINE UCActionComponent* GetActionComp() const { return ActionComponent; }
	FORCEINLINE UCApplyDamageComponent* GetApplyDamageComp() const { return ApplyDamageComponent; }
	FORCEINLINE UCTakeDamageComponent* GetTakeDamageComp() const { return TakeDamageComponent; }
	FORCEINLINE UCHealthComponent* GetHealthComp() const { return HealthComponent; }
	FORCEINLINE UCReactionComponent* GetReactionComp() const { return ReactionComponent; }

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

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

private:
	void ConsumePendingReaction();

private:
	bool CanActionInput() const;
};