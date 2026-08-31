#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Type/CMovementTypes.h"
#include "Type/CWeaponTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CCharacterFeedbackTypes.h"
#include "Type/CStateTypes.h"
#include "CAnimInstance.generated.h"

enum class EBalanceLifecycleState : uint8;

UCLASS()
class PORTFOLIO_API UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	// Animation Blueprint Parameters - Movement
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ELocomotionPresentationMode LocomotionPresentationMode = ELocomotionPresentationMode::Forward;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir = false;

protected:
	// Animation Blueprint Parameters - Weapon
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	EWeaponType CurrentWeaponType = EWeaponType::Max;

protected:
	// Animation Blueprint Parameters - State
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EExecutionState CurrentExecutionState = EExecutionState::Max;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDeadPose = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsCollapsePose = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsExecutionDownPose = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EDeathPresentationMode DeathPresentationMode = EDeathPresentationMode::Default;

protected:
	// Animation Blueprint Parameters - Action
	UPROPERTY(BlueprintReadOnly, Category = "Action")
	bool bIsGuardingPose = false;

private:
	// Cached Component References
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCDefenseComponent* DefenseComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCBalanceComponent* BalanceComp_Cached = nullptr;

private:
	// Runtime LOD Animation Refresh State
	float AnimationRefreshThrottleElapsedSeconds = 0.f;

public:
	// Lifecycle
	void NativeInitializeAnimation() override;
	void NativeUninitializeAnimation() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// Locomotion Presentation Query
	FORCEINLINE ELocomotionPresentationMode GetLocomotionPresentationMode() const { return LocomotionPresentationMode; }

private:
	// Reference Lifecycle
	bool CacheOwnerAndComponentReferences();
	void ClearCachedComponentReferences();
	void BindComponentEvents();
	void UnbindComponentEvents();

private:
	// Animation Parameter Lifecycle
	void RefreshMovementParameters();
	void RefreshStateParameters();
	void ResetAnimationParameters();

private:
	// Runtime LOD Animation Refresh Gate
	bool TryConsumeAnimationRefreshGate(float DeltaSeconds);
	void ResetAnimationRefreshThrottle();

private:
	// Animation Refresh Audit
	bool ShouldRecordAnimationRefreshAudit() const;

	void RecordAnimationRefreshAttempt() const;
	void RecordAnimationRefreshExecuted() const;
	void RecordAnimationRefreshSkipped() const;

private:
	// Component Event Handlers
	UFUNCTION()
	void HandleWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPreviousWeaponType, EWeaponType InCurrentWeaponType);
	void HandleDeadStateChanged(EDeadState InPreviousDeadState, EDeadState InCurrentDeadState);
	void HandleBalanceLifecycleStateChanged(EBalanceLifecycleState InPreviousState, EBalanceLifecycleState InCurrentState);
};
