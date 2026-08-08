#pragma once

#include "CoreMinimal.h"

class APawn;
class UWorld;
class UCDebugOverlayFocusComponent;

// ===== Public Runtime API =====

class PORTFOLIO_API FDebugOverlayFocusRuntimeHelper
{
public:
	static float GetNearestTargetRadius();
	static bool TryFocusNearestTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InRadius);
	static bool TryFocusOutlinerTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, const FString& InActorName);
	static bool TryFocusRecentCombatTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius);
	static void UpdateFocusRecentCombatTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius);
	static void ClearFocus(UCDebugOverlayFocusComponent* InFocusComponent);
};
