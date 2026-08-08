#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

class FDebugOverlayViewDataBuilder
{
public:
	static FDebugOverlayViewData Build(const UWorld* InWorld, const APawn* InViewerPawn, const ACEnemy* InDisplayEnemy, const FDebugOverlayFocusViewData& InEnemyFocus, const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting);
};
