#pragma once

#include "CoreMinimal.h"

class APawn;
class APlayerController;
class UWorld;
class UCDebugOverlayFocusComponent;

// ===== Public Runtime API =====

class PORTFOLIO_API FDebugOverlayFocusRuntimeHelper
{
public:
	static float GetNearestFocusRadius();
	static bool TryFocusNearestFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InRadius);
	static bool TryFocusOutlinerFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, const FString& InActorName);
	static bool TryFocusRecentCombatFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius);
	static bool TryFocusPlayerTarget(UCDebugOverlayFocusComponent* InFocusComponent, const APlayerController* InPlayerController);
	static void UpdateFocusRecentCombatFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius);
	static void UpdateFocusPlayerTarget(UCDebugOverlayFocusComponent* InFocusComponent, const APlayerController* InPlayerController);
	static void ClearFocus(UCDebugOverlayFocusComponent* InFocusComponent);
};
