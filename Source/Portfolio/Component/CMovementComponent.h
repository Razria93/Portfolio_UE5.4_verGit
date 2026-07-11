#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CMovementStructure.h"
#include "CMovementComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMovementComponent();

private:
	struct FRuntimeLODMovementState
	{
		int32 AppliedMode = INDEX_NONE;
		bool bOriginalMovementComponentTickEnabled = true;
		bool bOriginalStateCached = false;
	};

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
	FRuntimeLODMovementState RuntimeLODMovementState;

private:
	UPROPERTY(Transient)
	bool bCanMove = true;

	UPROPERTY(Transient)
	bool bRuntimeLODMovementIntentBlocked = false;

	UPROPERTY(Transient)
	bool bIsFalling = false;

private:
	UPROPERTY(Transient)
	float CurrentSpeed = 0.f;

	UPROPERTY(Transient)
	float CurrentDirection = 0.f;

private:
	/* === Injected Objects === */
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCharacterMovementComponent* CharacterMovementComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

protected:
	// Lifecycle
	void BeginPlay() override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Runtime LOD
	// 1. Update
	void UpdateRuntimeLODMovementMode();

	// 2. Lifecycle
	void EnsureRuntimeLODMovementOriginalStateCached();

	// 3. Dispatch
	void ApplyRuntimeLODMovementMode(int32 InMovementMode);

	// 4. Movement Mode
	void ApplyRuntimeLODMovementDefault();
	void ApplyRuntimeLODMovementStateRefreshDisabled();
	void ApplyRuntimeLODMovementIntentBlocked();

	// 5. Movement State
	void RestoreRuntimeLODMovementStateRefresh();
	void DisableRuntimeLODMovementStateRefresh();
	void AllowRuntimeLODMovementIntent();
	void BlockRuntimeLODMovementIntent();
	void StopRuntimeLODActiveMovement();

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
	FORCEINLINE void SetMove()
	{
		if (bRuntimeLODMovementIntentBlocked) return;

		bCanMove = true;
	}

public:
	/* === Movement Arbitration === */
	bool CanAcceptMoveInput() const;

	void BlockMovementIntentForRuntimeLOD();
	void ClearMovementIntentBlockForRuntimeLOD();

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
