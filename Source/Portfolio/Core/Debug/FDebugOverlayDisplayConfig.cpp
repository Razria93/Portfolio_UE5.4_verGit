#include "Core/Debug/FDebugOverlayDisplayConfig.h"

#include "HAL/IConsoleManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	// ===== Main Panel Section CVars =====

	TAutoConsoleVariable<int32> CVarDebugOverlayPlayerEnabled(TEXT("Portfolio.DebugOverlay.Player.Enabled"), 1, TEXT("Show the Player section in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayPlayerStatusEnabled(TEXT("Portfolio.DebugOverlay.Player.Status.Enabled"), 1, TEXT("Show Player status details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayPlayerTargetingEnabled(TEXT("Portfolio.DebugOverlay.Player.Targeting.Enabled"), 1, TEXT("Show Player targeting details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayPlayerLocomotionEnabled(TEXT("Portfolio.DebugOverlay.Player.Locomotion.Enabled"), 1, TEXT("Show Player locomotion details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayPlayerExecutionSessionEnabled(TEXT("Portfolio.DebugOverlay.Player.ExecutionSession.Enabled"), 1, TEXT("Show Player Execution Session details in Character Details. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayPlayerRecentActionReactionEnabled(TEXT("Portfolio.DebugOverlay.Player.RecentActionReaction.Enabled"), 1, TEXT("Show Player recent Action / Reaction details in Character Details. 0: hidden, 1: shown."), ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyEnabled(TEXT("Portfolio.DebugOverlay.Enemy.Enabled"), 1, TEXT("Show the Enemy section in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyFocusEnabled(TEXT("Portfolio.DebugOverlay.Enemy.Focus.Enabled"), 1, TEXT("Show Enemy focus details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyStatusEnabled(TEXT("Portfolio.DebugOverlay.Enemy.Status.Enabled"), 1, TEXT("Show Enemy status details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyBalanceCollapseEnabled(TEXT("Portfolio.DebugOverlay.Enemy.BalanceCollapse.Enabled"), 1, TEXT("Show Enemy Balance and Collapse details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyCombatTargetFacingEnabled(TEXT("Portfolio.DebugOverlay.Enemy.CombatTargetFacing.Enabled"), 1, TEXT("Show Enemy Combat Target Facing details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyExecutionSessionEnabled(TEXT("Portfolio.DebugOverlay.Enemy.ExecutionSession.Enabled"), 1, TEXT("Show Enemy Execution Session details in Character Details. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyCombatParticipationEnabled(TEXT("Portfolio.DebugOverlay.Enemy.CombatParticipation.Enabled"), 1, TEXT("Show Enemy Combat Participation details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyDeathLifecycleEnabled(TEXT("Portfolio.DebugOverlay.Enemy.DeathLifecycle.Enabled"), 1, TEXT("Show Enemy death lifecycle details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyRecentActionReactionEnabled(TEXT("Portfolio.DebugOverlay.Enemy.RecentActionReaction.Enabled"), 1, TEXT("Show Enemy recent Action / Reaction details in Character Details. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyCurrentAIEnabled(TEXT("Portfolio.DebugOverlay.Enemy.CurrentAI.Enabled"), 1, TEXT("Show Enemy current AI details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);
	TAutoConsoleVariable<int32> CVarDebugOverlayEnemyRecentAIEventEnabled(TEXT("Portfolio.DebugOverlay.Enemy.RecentAIEvent.Enabled"), 1, TEXT("Show Enemy recent AI event details in the Debug Overlay main panel. 0: hidden, 1: shown."), ECVF_Default);

	// ===== World Summary Section CVars =====

	TAutoConsoleVariable<int32> CVarDebugOverlayWorldSummaryCombatParticipationEnabled(TEXT("Portfolio.DebugOverlay.WorldSummary.CombatParticipation.Enabled"), 1, TEXT("Show Combat Participation in the Debug Overlay World Summary panel. 0: hidden, 1: shown."), ECVF_Default);
}
#endif

FDebugOverlayPanelVisibility DebugOverlayDisplayConfig::GetPanelVisibility()
{
	FDebugOverlayPanelVisibility visibility;

#if !UE_BUILD_SHIPPING
	visibility.bShowPlayer = CVarDebugOverlayPlayerEnabled.GetValueOnGameThread() != 0;
	visibility.bShowPlayerStatus = CVarDebugOverlayPlayerStatusEnabled.GetValueOnGameThread() != 0;
	visibility.bShowPlayerTargeting = CVarDebugOverlayPlayerTargetingEnabled.GetValueOnGameThread() != 0;
	visibility.bShowPlayerLocomotion = CVarDebugOverlayPlayerLocomotionEnabled.GetValueOnGameThread() != 0;
	visibility.bShowPlayerExecutionSession = CVarDebugOverlayPlayerExecutionSessionEnabled.GetValueOnGameThread() != 0;
	visibility.bShowPlayerRecentActionReaction = CVarDebugOverlayPlayerRecentActionReactionEnabled.GetValueOnGameThread() != 0;

	visibility.bShowEnemy = CVarDebugOverlayEnemyEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyFocus = CVarDebugOverlayEnemyFocusEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyStatus = CVarDebugOverlayEnemyStatusEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyBalanceCollapse = CVarDebugOverlayEnemyBalanceCollapseEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyCombatTargetFacing = CVarDebugOverlayEnemyCombatTargetFacingEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyExecutionSession = CVarDebugOverlayEnemyExecutionSessionEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyCombatParticipation = CVarDebugOverlayEnemyCombatParticipationEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyDeathLifecycle = CVarDebugOverlayEnemyDeathLifecycleEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyRecentActionReaction = CVarDebugOverlayEnemyRecentActionReactionEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyCurrentAI = CVarDebugOverlayEnemyCurrentAIEnabled.GetValueOnGameThread() != 0;
	visibility.bShowEnemyRecentAIEvent = CVarDebugOverlayEnemyRecentAIEventEnabled.GetValueOnGameThread() != 0;
	visibility.bShowWorldSummaryCombatParticipation = CVarDebugOverlayWorldSummaryCombatParticipationEnabled.GetValueOnGameThread() != 0;
#endif

	return visibility;
}
