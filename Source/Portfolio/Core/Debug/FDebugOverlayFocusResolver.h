#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlayFocusTypes.h"

class APawn;
class UWorld;

enum class EDebugOverlayFocusResolveOutcome : uint8
{
	Selected,
	
	NoTarget,
	NoRecentCombatEvidence,
	OutOfRange,
	
	InvalidContext,
	NoActor,
	NoActorName,
	TargetIsNotEnemy,
};

struct FDebugOverlayFocusResolveResult
{
	TWeakObjectPtr<AActor> FocusActor;
	EDebugOverlayFocusSource Source = EDebugOverlayFocusSource::None;
	EDebugOverlayFocusResolveOutcome Outcome = EDebugOverlayFocusResolveOutcome::InvalidContext;
	FString ActorName;
	FString ClassName;
	float Distance = 0.f;
	float Radius = 0.f;
};

class PORTFOLIO_API FDebugOverlayFocusResolver
{
public:
	static FDebugOverlayFocusResolveResult ResolveNearestTarget(UWorld* World, const APawn* ViewerPawn, float Radius);
	static FDebugOverlayFocusResolveResult ResolveOutlinerTarget(UWorld* World, const APawn* ViewerPawn, const FString& ActorName);
	static FDebugOverlayFocusResolveResult ResolveRecentCombatTarget(UWorld* World, const APawn* ViewerPawn, float FallbackRadius);
};
