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
	TMap<class AActor*, FTimerHandle> ActiveHitStopMap;
	TMap<class AActor*, float> CachedTimeDilationMap;

public:
	FOnCombatCameraShakeRequested OnCameraShakeRequested;

public:
	void RequestHitStop(const FHitStopRequest& InHitStopRequest);
	void RequestCameraShake(const FCameraShakeRequest& InCameraShakeRequest);

private:
	void ApplyHitStop(AActor* InActor, float InDuration, float InDilation);
	void RestoreHitStop(AActor* InActor);

private:
	void PrintHitStopRuntimeInfo(AActor* InActor, float InDuration, float InDilation) const;
	void PrintCameraShakeRuntimeInfo(const FCameraShakeRequest& InCameraShakeRequest) const;
};
