#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Type/CWeaponStructure.h"
#include "CAnimInstance.generated.h"

UCLASS()
class PORTFOLIO_API UCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsInAir = false;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	EWeaponType CurrentWeaponType = EWeaponType::Max;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	EDeadState DeadState = EDeadState::Alive;

	UPROPERTY(BlueprintReadOnly, Category = "Action")
	bool bIsGuardingPose = false;

private:
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

private:
	float RuntimeLODAnimationRefreshElapsed = 0.f;

public:
	// Lifecycle
	void NativeInitializeAnimation() override;
	void NativeUninitializeAnimation() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	// Reference Cache
	bool CacheOwnerAndComponents();
	void ClearCachedReferences();
	void BindComponentEvents();
	void UnbindComponentEvents();

private:
	// Animation Profiling Gate
	void InitializeAnimationStateForProfiling();
	void ClearAnimationStateForProfiling();

	// Condition
	bool ShouldReduceEnemyAnimationRefreshForProfiling() const;
	bool IsEnemyAnimationProfilingTarget() const;

	// Query
	float GetReducedAnimationRefreshIntervalForProfiling() const;

	// Gate
	bool ShouldRefreshAnimationParameters(float DeltaSeconds);

private:
	// Animation Refresh Audit
	bool ShouldAuditAnimationRefreshForProfiling() const;

	// Record
	void RecordAnimationRefreshAttemptForProfiling() const;
	void RecordAnimationRefreshExecutedForProfiling() const;
	void RecordAnimationRefreshSkippedForProfiling() const;

private:
	// Parameter Refresh
	void RefreshMovementParameters();
	void RefreshStateParameters();

private:
	// Component Event Callback
	UFUNCTION()
	void OnWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPrevWeaponType, EWeaponType InNewWeaponType);
};
