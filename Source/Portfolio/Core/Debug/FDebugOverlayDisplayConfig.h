#pragma once

#include "CoreMinimal.h"

struct FDebugOverlayPanelVisibility
{
	bool bShowPlayer = true;
	bool bShowPlayerStatus = true;
	bool bShowPlayerTargeting = true;
	bool bShowPlayerLocomotion = true;
	bool bShowPlayerExecutionSession = true;
	bool bShowPlayerRecentActionReaction = true;

	bool bShowEnemy = true;
	bool bShowEnemyFocus = true;
	bool bShowEnemyStatus = true;
	bool bShowEnemyBalanceCollapse = true;
	bool bShowEnemyCombatTargetFacing = true;
	bool bShowEnemyExecutionSession = true;
	bool bShowEnemyCombatParticipation = true;
	bool bShowEnemyDeathLifecycle = true;
	bool bShowEnemyRecentActionReaction = true;
	bool bShowEnemyCurrentAI = true;
	bool bShowEnemyRecentAIEvent = true;

	bool bShowWorldSummaryCombatParticipation = true;
};

namespace DebugOverlayDisplayConfig
{
	FDebugOverlayPanelVisibility GetPanelVisibility();
}
