#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Type/CWorldSubSystemStructure.h"
#include "CWorldSubsystem_CombatFeedback.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatCameraShakeRequested, const FCameraShakeRequest& InCameraShakeRequest);

UCLASS()
class PORTFOLIO_API UCWorldSubsystem_CombatFeedback : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	TMap<TWeakObjectPtr<class AActor>, FTimerHandle> ActiveHitStopMap;
	TMap<TWeakObjectPtr<class AActor>, float> CachedTimeDilationMap;

public:
	// Delegate
	FOnCombatCameraShakeRequested OnCameraShakeRequested;

public:
	// Lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// Request
	void RequestHitStop(const FHitStopRequest& InHitStopRequest);
	void RequestCameraShake(const FCameraShakeRequest& InCameraShakeRequest);

private:
	// HitStop
	void ApplyHitStop(AActor* InActor, float InDuration, float InDilation);
	void RestoreHitStop(TWeakObjectPtr<AActor> InActorKey);
	void ClearHitStop();

private:
	// CameraShake
	void ClearCameraShake();

private:
	// Runtime State
	void ClearFeedbackRuntimeState();

private:
	// Debug
	void PrintHitStopConsumeInfo(AActor* InActor, float InDuration, float InDilation) const;
};
