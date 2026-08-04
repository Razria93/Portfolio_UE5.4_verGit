#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/CDebugOverlayFocusComponent.h"

class APawn;
class UWorld;

enum class EDebugOverlayFocusResolveStatus : uint8
{
	Selected,
	InvalidContext,
	NoEnemy,
	NoRecentCombat,
	OutOfRange,
	NoActorName,
	NoActor,
	NotEnemy,
};

struct FDebugOverlayFocusResolveResult
{
	EDebugOverlayFocusResolveStatus Status = EDebugOverlayFocusResolveStatus::InvalidContext;
	TWeakObjectPtr<AActor> FocusActor;
	EDebugOverlayFocusSource FocusSource = EDebugOverlayFocusSource::None;
	FString ActorName;
	FString ClassName;
	float Distance = 0.f;
	float Radius = 0.f;
};

class PORTFOLIO_API FDebugOverlayFocusResolver
{
public:
	static FDebugOverlayFocusResolveResult ResolveNearestEnemy(
		UWorld* World,
		const APawn* ViewerPawn,
		float Radius);

	static FDebugOverlayFocusResolveResult ResolveActorEnemy(
		UWorld* World,
		const APawn* ViewerPawn,
		const FString& ActorName);

	static FDebugOverlayFocusResolveResult ResolveRecentCombatEnemy(
		UWorld* World,
		const APawn* ViewerPawn,
		float FallbackRadius);
};
