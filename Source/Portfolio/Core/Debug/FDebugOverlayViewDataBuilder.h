#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlayDisplayConfig.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

class FDebugOverlayViewDataBuilder
{
public:
	static FDebugOverlayViewData Build(
		const UWorld* InWorld,
		const APawn* InViewerPawn,
		const ACEnemy* InDisplayEnemy,
		const FDebugOverlayFocusViewData& InEnemyFocus,
		const FDebugOverlayPlayerTargetingViewData& InPlayerTargeting,
		const FDebugOverlayPlayerLocomotionViewData& InPlayerLocomotion,
		const FDebugOverlayExecutionSessionViewData& InPlayerExecutionSession,
		const FDebugOverlayBalanceCollapseViewData& InBalanceCollapse,
		const FDebugOverlayCombatTargetFacingViewData& InCombatTargetFacing,
		const FDebugOverlayExecutionSessionViewData& InEnemyExecutionSession,
		const FDebugOverlayCombatParticipationViewData& InCombatParticipation,
		const FDebugOverlayPanelVisibility& InPanelVisibility);
};
