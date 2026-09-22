#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Type/CCombatFeedbackTypes.h"
#include "CWorldSubsystem_CombatFeedback.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatCameraShakeRequested, const FCameraShakeRequest& InCameraShakeRequest);

UCLASS()
class PORTFOLIO_API UCWorldSubsystem_CombatFeedback : public UTickableWorldSubsystem
{
	GENERATED_BODY()

private:
	struct FTimeEffect
	{
		float Dilation = 1.f;
		double ExpiresAt = 0.;
		bool bRealTime = false;
		uint64 OwnerSerial = 0;
	};
	struct FActorTimeEffects
	{
		float Baseline = 1.f;
		TArray<FTimeEffect> Effects;
	};
	TMap<TWeakObjectPtr<class AActor>, FActorTimeEffects> ActorTimeEffects;
	uint64 NextSlowSerial = 0;

public:
	// Delegate
	FOnCombatCameraShakeRequested OnCameraShakeRequested;

public:
	// Lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	void Tick(float InDeltaTime) override;
	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(CombatFeedback, STATGROUP_Tickables); }

public:
	// Request
	void RequestHitStop(const FHitStopRequest& InHitStopRequest);
	void RequestCameraShake(const FCameraShakeRequest& InCameraShakeRequest);
	uint64 RequestPerfectDodgeSlow(AActor* InPlayer, AActor* InAttacker, float InDuration, float InDilation);
	void CancelPerfectDodgeSlow(uint64 InSerial);

private:
	// HitStop
	void ApplyHitStop(AActor* InActor, float InDuration, float InDilation);
	void AddTimeEffect(AActor* InActor, float InDuration, float InDilation, bool bRealTime, uint64 InSerial);
	void RefreshTimeEffects();
	void ClearHitStop();

private:
	// Camera Shake
	void ClearCameraShake();

private:
	// Runtime State
	void ClearFeedbackRuntimeState();
};
