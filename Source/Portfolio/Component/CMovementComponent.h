#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CMovementTypes.h"
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

private:
	UPROPERTY(EditAnywhere, Category = "Movement|Gait")
	TMap<EMovementGait, float> GaitSpeedMap;

private:
	UPROPERTY(Transient)
	EMovementGait CurrentMovementGait = EMovementGait::Run;

	UPROPERTY(Transient)
	EMovementGait CachedMovementGait_BeforeOverride = EMovementGait::Run;

	UPROPERTY(Transient)
	bool bHasMovementGaitOverride = false;

	UPROPERTY(Transient)
	EMovementRotationMode CurrentMovementRotationMode = EMovementRotationMode::OrientToMovement;

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
	void UpdateRuntimeLODMovementMode();
	void EnsureRuntimeLODMovementOriginalStateCached();
	void ApplyRuntimeLODMovementMode(int32 InMovementMode);
	void ApplyRuntimeLODMovementDefault();
	void ApplyRuntimeLODMovementStateRefreshDisabled();
	void ApplyRuntimeLODMovementIntentBlocked();
	void RestoreRuntimeLODMovementStateRefresh();
	void DisableRuntimeLODMovementStateRefresh();
	void AllowRuntimeLODMovementIntent();
	void BlockRuntimeLODMovementIntent();
	void StopRuntimeLODActiveMovement();

public:
	// Query
	FORCEINLINE bool CheckCurrentMovementGait(EMovementGait InNewMovementGait) const { return CurrentMovementGait == InNewMovementGait; }
	FORCEINLINE EMovementGait GetCurrentMovementGait() const { return CurrentMovementGait; }
	FORCEINLINE EMovementRotationMode GetCurrentMovementRotationMode() const { return CurrentMovementRotationMode; }

	FORCEINLINE bool CanMove() const { return bCanMove; }
	FORCEINLINE bool IsFalling() const { return bIsFalling; }
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	FORCEINLINE float GetCurrentDirection() const { return CurrentDirection; }

public:
	// Mutation
	FORCEINLINE void SetStop() { bCanMove = false; }
	FORCEINLINE void SetMove()
	{
		if (bRuntimeLODMovementIntentBlocked) return;

		bCanMove = true;
	}

public:
	// Movement Arbitration
	bool CanAcceptMoveInput() const;

	void BlockMovementIntentForRuntimeLOD();
	void ClearMovementIntentBlockForRuntimeLOD();

public:
	// Movement Input
	void OnMove(const FVector2D& InAxis2D);

public:
	void OnWalk();
	void OnRun();
	void OnSprint();

public:
	void OnJump();
	void OnStopJump();

public:
	// Movement Policy
	void ApplyMovementGaitOverride(EMovementGait InGait);
	void ClearMovementGaitOverride();
	void SetMovementRotationMode(EMovementRotationMode InRotationMode);

private:
	void SetMovementGait(EMovementGait InNewMovementGait);
	void ApplyMovementGait(EMovementGait InNewMovementGait);

private:
	void ApplyMovementRotationMode(EMovementRotationMode InRotationMode);

private:
	void CalculateSpeed();
	void CalculateDirection();
};
