#pragma once

#include "CoreMinimal.h"

struct FDebugOverlayPanelVisibility
{
	bool bShowPlayer = true;
	bool bShowPlayerStatus = true;
	bool bShowPlayerTargeting = true;
	bool bShowPlayerLocomotion = true;
	bool bShowPlayerExecutionCollaboration = true;
	bool bShowPlayerRecentExecution = true;

	bool bShowEnemy = true;
	bool bShowEnemyFocus = true;
	bool bShowEnemyStatus = true;
	bool bShowEnemyBalanceCollapse = true;
	bool bShowEnemyCombatTargetFacing = true;
	bool bShowEnemyExecutionCollaboration = true;
	bool bShowEnemyCombatParticipation = true;
	bool bShowEnemyDeathLifecycle = true;
	bool bShowEnemyRecentExecution = true;
	bool bShowEnemyCurrentAI = true;
	bool bShowEnemyRecentAIEvent = true;

	bool bShowWorldSummaryCombatParticipation = true;
};

namespace DebugOverlayDisplayConfig
{
	FDebugOverlayPanelVisibility GetPanelVisibility();
}
