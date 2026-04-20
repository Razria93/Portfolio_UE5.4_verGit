#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CMovementStructure.h"
#include "CMovementComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMovementComponent();

	// === MovementData ===================================== //
private:
	UPROPERTY(EditAnywhere, Category = "Movement|Gait")
	TMap<EMovementGait, float> GaitSpeedMap;

	// ====================================================== //

private:
	/* === State === */
	UPROPERTY(Transient)
	EMovementGait CurrentMovementGait = EMovementGait::Run;

private:
	UPROPERTY(Transient)
	bool bCanMove = true;

	UPROPERTY(Transient)
	bool bIsFalling = false;

private:
	UPROPERTY(Transient)
	float CurrentSpeed = 0.f;

	UPROPERTY(Transient)
	float CurrentDirection = 0.f;

private:
	/* === Cached Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCharacterMovementComponent* CharacterMovementComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

protected:
	void BeginPlay() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/* === Check / Query === */
	FORCEINLINE bool CanMove() const { return bCanMove; }
	FORCEINLINE bool IsFalling() const { return bIsFalling; }

public:
	/* === Getter === */
	FORCEINLINE EMovementGait GetCurrentMovementGait() const { return CurrentMovementGait; }

public:
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	FORCEINLINE float GetCurrentDirection() const { return CurrentDirection; }

public:
	/* === Setter === */
	FORCEINLINE void SetStop() { bCanMove = false; }
	FORCEINLINE void SetMove() { bCanMove = true; }

public:
	void OnMoveForward(float InValue);
	void OnMoveRight(float InValue);

public:
	void OnWalk();
	void OnRun();
	void OnSprint();

public:
	void OnJump();
	void OnStopJump();

public:
	bool CanAcceptMoveInput() const;

private:
	void ChangeMovementGait(EMovementGait InNewMovementGait);

private:
	void CalculateSpeed();
	void CalculateDirection();
};