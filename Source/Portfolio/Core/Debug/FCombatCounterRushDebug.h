#pragma once

#include "CoreMinimal.h"
#include "Type/CCounterRushTypes.h"

/** Per-executor bounded diagnostic; no global actor registry or gameplay decisions. */
struct PORTFOLIO_API FCombatCounterRushDebug
{
	FCounterRushPlan Plan;
	TArray<FVector> Samples;
	uint64 Serial = 0;
	float PathDistance = 0.f;
	float Duration = 0.f;
	bool bCapturing = false;
	void Begin(const AActor* InActor, const FCounterRushPlan& InPlan, uint64 InSerial, float InDuration);
	void Sample(const AActor* InActor);
	void End(const AActor* InActor, const TCHAR* InReason);
private:
	void Draw(const AActor* InActor, const TCHAR* InReason, float InLifetime) const;
};
