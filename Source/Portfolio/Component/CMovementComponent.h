#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CMovementTypes.h"
#include "CMovementComponent.generated.h"

enum class EBalanceLifecycleState : uint8;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Construction
	UCMovementComponent();

private:
	// Runtime Type
	struct FRuntimeLODMovementState
	{
		int32 AppliedMode = INDEX_NONE;
	};

private:
	// Config
	UPROPERTY(EditAnywhere, Category = "Movement|Gait")
	TMap<EMovementGait, float> GaitSpeedMap;

private:
	// Locomotion Policy Runtime
	UPROPERTY(Transient)
	EMovementGait CurrentMovementGait = EMovementGait::Run;

	UPROPERTY(Transient)
	EMovementGait CachedMovementGaitBeforeOverride = EMovementGait::Run;

	UPROPERTY(Transient)
	bool bHasMovementGaitOverride = false;

	UPROPERTY(Transient)
	EMovementRotationMode CurrentMovementRotationMode = EMovementRotationMode::OrientToMovement;

private:
	// Gameplay Movement Gate
	UPROPERTY(Transient)
	bool bIsMovementEnabled = true;

	// Runtime LOD Overlay Gate
	UPROPERTY(Transient)
	bool bRuntimeLODMovementIntentBlocked = false;

	FRuntimeLODMovementState RuntimeLODMovementState;

	// Observed Movement State
	UPROPERTY(Transient)
	bool bIsFalling = false;
	UPROPERTY(Transient)
	float CurrentSpeed = 0.f;

	UPROPERTY(Transient)
	float CurrentDirection = 0.f;

private:
	// Component References
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCharacterMovementComponent* CharacterMovementComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCBalanceComponent* BalanceComp_Injected = nullptr;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

protected:
	// Lifecycle
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;
	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
public:
	// Query: Locomotion State
	FORCEINLINE EMovementGait GetCurrentMovementGait() const { return CurrentMovementGait; }
	FORCEINLINE EMovementRotationMode GetCurrentMovementRotationMode() const { return CurrentMovementRotationMode; }

	FORCEINLINE bool IsFalling() const { return bIsFalling; }
	FORCEINLINE float GetCurrentSpeed() const { return CurrentSpeed; }
	FORCEINLINE float GetCurrentDirection() const { return CurrentDirection; }

public:
	// Query: Movement Arbitration
	FORCEINLINE bool IsMovementEnabled() const { return bIsMovementEnabled; }
	bool CanAcceptMovementIntent() const;

public:
	// Gameplay Movement Permission
	void SetMovementEnabled(bool bEnabled);

public:
	// Movement Input Handling
	void HandleMoveInput(const FVector2D& InAxis2D);
	void HandleWalkInput();
	void HandleRunInput();
	void HandleSprintInput();
	void HandleJumpInput();
	void HandleJumpInputReleased();

public:
	// Movement Policy
	void ApplyMovementGaitOverride(EMovementGait InGait);
	void ClearMovementGaitOverride();
	void SetMovementRotationMode(EMovementRotationMode InRotationMode);

private:
	// Component Reference Validation
	bool ValidateRequiredComponentReferences() const;

private:
	// Runtime LOD Update
	void UpdateRuntimeLODMovementMode();
	void ApplyRuntimeLODMovementMode(int32 InMovementMode);
	void SetRuntimeLODMovementIntentBlocked(bool bBlocked);
	void StopActiveAIMovement();

private:
	// Balance Lifecycle Event
	void HandleBalanceLifecycleStateChanged(EBalanceLifecycleState InPreviousState, EBalanceLifecycleState InNewState);

private:
	// Gait Implementation
	void SetMovementGait(EMovementGait InNewMovementGait);
	void ApplyMovementGait(EMovementGait InNewMovementGait);

private:
	// Rotation Implementation
	void ApplyMovementRotationMode(EMovementRotationMode InRotationMode);

private:
	// Runtime State Refresh
	void CalculateSpeed();
	void CalculateDirection();
};
