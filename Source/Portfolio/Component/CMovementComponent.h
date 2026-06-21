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

	UPROPERTY(Transient)
	EMovementGait CachedMovementGait_BeforeOverride = EMovementGait::Run;

	UPROPERTY(Transient)
	bool bHasMovementModeOverride = false;

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
	FORCEINLINE bool CheckCurrentMovementGait(EMovementGait InNewMovementGait) const { return CurrentMovementGait == InNewMovementGait; }

public:
	/* === Getter === */
	FORCEINLINE EMovementGait GetCurrentMovementGait() const { return CurrentMovementGait; }

public:
	FORCEINLINE bool CanMove() const { return bCanMove; }
	FORCEINLINE bool IsFalling() const { return bIsFalling; }
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	FORCEINLINE float GetCurrentDirection() const { return CurrentDirection; }

public:
	/* === Setter === */
	FORCEINLINE void SetStop() { bCanMove = false; }
	FORCEINLINE void SetMove() { bCanMove = true; }

public:
	/* === Movement Arbitration === */
	bool CanAcceptMoveInput() const;

public:
	void OnMove(const FVector2D& InAxis2D);

public:
	void OnWalk();
	void OnRun();
	void OnSprint();

public:
	void OnJump();
	void OnStopJump();

public:
	/* === Movement Policy === */
	void ApplyMovementOverride(EMovementGait InGait, EMovementRotationMode InRotationMode);
	void ClearMovementOverride();

private:
	void ChangeMovementGait(EMovementGait InNewMovementGait);
	void ApplyMovementGait(EMovementGait InNewMovementGait);

private:
	void ApplyRotationMode(EMovementRotationMode InRotationMode);

private:
	void CalculateSpeed();
	void CalculateDirection();
};
