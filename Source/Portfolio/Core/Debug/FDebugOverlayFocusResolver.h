#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/CDebugOverlayTargetComponent.h"

class APawn;
class UWorld;

enum class EDebugOverlayFocusResolveStatus : uint8
{
	Selected,
	InvalidContext,
	NoEnemy,
	OutOfRange,
	NoActorName,
	NoActor,
	NotEnemy,
};

struct FDebugOverlayFocusResolveResult
{
	EDebugOverlayFocusResolveStatus Status = EDebugOverlayFocusResolveStatus::InvalidContext;
	TWeakObjectPtr<AActor> FocusActor;
	EDebugOverlayTargetSource FocusSource = EDebugOverlayTargetSource::None;
	FString SummaryText;
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
};
