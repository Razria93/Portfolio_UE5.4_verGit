#include "FPortfolioDebugOverlayEditorCVarAccess.h"

#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "FPortfolioDebugOverlayEditorCVarAccess"

namespace
{
	// ===== Overlay CVar Names =====

	static constexpr const TCHAR* DebugOverlayEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enabled");
	static constexpr const TCHAR* DebugOverlayCollectCVarName = TEXT("Portfolio.DebugOverlay.Collect");
	static constexpr const TCHAR* DebugOverlayEventLogFilterCVarName = TEXT("Portfolio.DebugOverlay.EventLogFilter");
	static constexpr const TCHAR* DebugOverlayEventLogLimitCVarName = TEXT("Portfolio.DebugOverlay.EventLogLimit");
	static constexpr const TCHAR* DebugOverlayHideNoiseEventsCVarName = TEXT("Portfolio.DebugOverlay.HideNoiseEvents");
	static constexpr const TCHAR* DebugOverlayHideCollisionWindowEventsCVarName = TEXT("Portfolio.DebugOverlay.HideCollisionWindowEvents");
	static constexpr const TCHAR* DeathLifecycleAuditCVarName = TEXT("Portfolio.Debug.DeathLifecycleAudit");
	static constexpr const TCHAR* DebugOverlayNearestFocusRadiusCVarName = TEXT("Portfolio.DebugOverlay.NearestFocusRadius");

	// ===== Main Panel Section CVar Names =====

	static constexpr const TCHAR* DebugOverlayPlayerPanelEnabledCVarName = TEXT("Portfolio.DebugOverlay.Player.Enabled");
	static constexpr const TCHAR* DebugOverlayPlayerStatusEnabledCVarName = TEXT("Portfolio.DebugOverlay.Player.Status.Enabled");
	static constexpr const TCHAR* DebugOverlayPlayerTargetingEnabledCVarName = TEXT("Portfolio.DebugOverlay.Player.Targeting.Enabled");
	static constexpr const TCHAR* DebugOverlayPlayerLocomotionEnabledCVarName = TEXT("Portfolio.DebugOverlay.Player.Locomotion.Enabled");
	static constexpr const TCHAR* DebugOverlayPlayerRecentExecutionEnabledCVarName = TEXT("Portfolio.DebugOverlay.Player.RecentExecution.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyPanelEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyFocusEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.Focus.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyStatusEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.Status.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyBalanceCollapseEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.BalanceCollapse.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyCombatParticipationEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.CombatParticipation.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyDeathLifecycleEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.DeathLifecycle.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyRecentExecutionEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.RecentExecution.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyCurrentAIEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.CurrentAI.Enabled");
	static constexpr const TCHAR* DebugOverlayEnemyRecentAIEventEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enemy.RecentAIEvent.Enabled");
	static constexpr const TCHAR* DebugOverlayWorldSummaryCombatParticipationEnabledCVarName = TEXT("Portfolio.DebugOverlay.WorldSummary.CombatParticipation.Enabled");

	// ===== Targeting Display CVar Names =====

	static constexpr const TCHAR* DebugOverlayTargetingEnabledCVarName = TEXT("Portfolio.DebugOverlay.Targeting.Enabled");
	static constexpr const TCHAR* DebugOverlayTargetingDrawRangeSphereCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawRangeSphere");
	static constexpr const TCHAR* DebugOverlayTargetingDrawSelectedTargetSphereCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawSelectedTargetSphere");
	static constexpr const TCHAR* DebugOverlayTargetingDrawViewLineCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawViewLine");
	static constexpr const TCHAR* DebugOverlayTargetingDrawDebugTextCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawDebugText");

	// ===== Movement Display CVar Names =====

	static constexpr const TCHAR* DebugOverlayMovementEnabledCVarName = TEXT("Portfolio.DebugOverlay.Movement.Enabled");
	static constexpr const TCHAR* DebugOverlayMovementDrawVelocityCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawVelocity");
	static constexpr const TCHAR* DebugOverlayMovementDrawInputCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawInput");
	static constexpr const TCHAR* DebugOverlayMovementDrawFacingCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawFacing");
	static constexpr const TCHAR* DebugOverlayMovementDrawDebugTextCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawDebugText");

	// ===== Balance Display CVar Names =====

	static constexpr const TCHAR* DebugOverlayBalanceEnabledCVarName = TEXT("Portfolio.DebugOverlay.Balance.Enabled");
	static constexpr const TCHAR* DebugOverlayBalanceDrawWorldTextCVarName = TEXT("Portfolio.DebugOverlay.Balance.DrawWorldText");
	static constexpr const TCHAR* DebugOverlayBalanceDrawLifecycleBarCVarName = TEXT("Portfolio.DebugOverlay.Balance.DrawLifecycleBar");
	static constexpr const TCHAR* BalanceAuditCVarName = TEXT("Portfolio.Debug.BalanceAudit");

	// ===== Combat Participation Display CVar Names =====

	static constexpr const TCHAR* DebugOverlayCombatParticipationEnabledCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled");
	static constexpr const TCHAR* DebugOverlayCombatParticipationDrawWorldTextCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldText");
	static constexpr const TCHAR* DebugOverlayCombatParticipationDrawWorldRingCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldRing");
	static constexpr const TCHAR* DebugOverlayCombatParticipationDrawHitReactiveEvidenceAnchorCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawHitReactiveEvidenceAnchor");

	// ===== Focus CVar Names =====

	static constexpr const TCHAR* DebugOverlayFocusLiveSyncPlayerTargetCVarName = TEXT("Portfolio.DebugOverlay.Focus.LiveSyncPlayerTarget");

	// ===== Generic Lookup =====

	IConsoleVariable* FindConsoleVariable(const TCHAR* InName)
	{
		if (!InName) return nullptr;

		// Console variables can be unregistered and recreated during module reload.
		// Do not retain their raw pointers across Slate attribute evaluations.
		return IConsoleManager::Get().FindConsoleVariable(InName);
	}
}

// ===== Overlay CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnabledCVarName()
{
	return DebugOverlayEnabledCVarName;
}

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCollectCVarName()
{
	return DebugOverlayCollectCVarName;
}

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEventLogFilterCVarName()
{
	return DebugOverlayEventLogFilterCVarName;
}

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEventLogLimitCVarName()
{
	return DebugOverlayEventLogLimitCVarName;
}

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetHideNoiseEventsCVarName()
{
	return DebugOverlayHideNoiseEventsCVarName;
}

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetHideCollisionWindowEventsCVarName()
{
	return DebugOverlayHideCollisionWindowEventsCVarName;
}

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetDeathLifecycleAuditCVarName()
{
	return DeathLifecycleAuditCVarName;
}

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetNearestFocusRadiusCVarName()
{
	return DebugOverlayNearestFocusRadiusCVarName;
}

// ===== Main Panel Section CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetPlayerPanelEnabledCVarName() { return DebugOverlayPlayerPanelEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetPlayerStatusEnabledCVarName() { return DebugOverlayPlayerStatusEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetPlayerTargetingEnabledCVarName() { return DebugOverlayPlayerTargetingEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetPlayerLocomotionEnabledCVarName() { return DebugOverlayPlayerLocomotionEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetPlayerRecentExecutionEnabledCVarName() { return DebugOverlayPlayerRecentExecutionEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyPanelEnabledCVarName() { return DebugOverlayEnemyPanelEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyFocusEnabledCVarName() { return DebugOverlayEnemyFocusEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyStatusEnabledCVarName() { return DebugOverlayEnemyStatusEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyBalanceCollapseEnabledCVarName() { return DebugOverlayEnemyBalanceCollapseEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyCombatParticipationEnabledCVarName() { return DebugOverlayEnemyCombatParticipationEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyDeathLifecycleEnabledCVarName() { return DebugOverlayEnemyDeathLifecycleEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyRecentExecutionEnabledCVarName() { return DebugOverlayEnemyRecentExecutionEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyCurrentAIEnabledCVarName() { return DebugOverlayEnemyCurrentAIEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetEnemyRecentAIEventEnabledCVarName() { return DebugOverlayEnemyRecentAIEventEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetWorldSummaryCombatParticipationEnabledCVarName() { return DebugOverlayWorldSummaryCombatParticipationEnabledCVarName; }

// ===== Targeting Display CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingEnabledCVarName() { return DebugOverlayTargetingEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawRangeSphereCVarName() { return DebugOverlayTargetingDrawRangeSphereCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawSelectedTargetSphereCVarName() { return DebugOverlayTargetingDrawSelectedTargetSphereCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawViewLineCVarName() { return DebugOverlayTargetingDrawViewLineCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawDebugTextCVarName() { return DebugOverlayTargetingDrawDebugTextCVarName; }
// ===== Movement Display CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementEnabledCVarName() { return DebugOverlayMovementEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawVelocityCVarName() { return DebugOverlayMovementDrawVelocityCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawInputCVarName() { return DebugOverlayMovementDrawInputCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawFacingCVarName() { return DebugOverlayMovementDrawFacingCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawDebugTextCVarName() { return DebugOverlayMovementDrawDebugTextCVarName; }

// ===== Balance Display CVars =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetBalanceEnabledCVarName() { return DebugOverlayBalanceEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetBalanceDrawWorldTextCVarName() { return DebugOverlayBalanceDrawWorldTextCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetBalanceDrawLifecycleBarCVarName() { return DebugOverlayBalanceDrawLifecycleBarCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetBalanceAuditCVarName() { return BalanceAuditCVarName; }

// ===== Combat Participation Display CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationEnabledCVarName() { return DebugOverlayCombatParticipationEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationDrawWorldTextCVarName() { return DebugOverlayCombatParticipationDrawWorldTextCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationDrawWorldRingCVarName() { return DebugOverlayCombatParticipationDrawWorldRingCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationDrawHitReactiveEvidenceAnchorCVarName() { return DebugOverlayCombatParticipationDrawHitReactiveEvidenceAnchorCVarName; }

// ===== Focus CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetFocusLiveSyncPlayerTargetCVarName() { return DebugOverlayFocusLiveSyncPlayerTargetCVarName; }

// ===== CVar Access =====

IConsoleVariable* PortfolioDebugOverlayEditorCVarAccess::FindCVar(const TCHAR* InName)
{
	return FindConsoleVariable(InName);
}

bool PortfolioDebugOverlayEditorCVarAccess::GetBool(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		return consoleVariable->GetInt() != 0;
	}

	return false;
}

void PortfolioDebugOverlayEditorCVarAccess::SetBool(const TCHAR* InName, bool bInValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		consoleVariable->Set(bInValue ? 1 : 0, ECVF_SetByConsole);
	}
}

int32 PortfolioDebugOverlayEditorCVarAccess::GetInt(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		return consoleVariable->GetInt();
	}

	return 0;
}

void PortfolioDebugOverlayEditorCVarAccess::SetInt(const TCHAR* InName, int32 InValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		consoleVariable->Set(InValue, ECVF_SetByConsole);
	}
}

float PortfolioDebugOverlayEditorCVarAccess::GetFloat(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		return consoleVariable->GetFloat();
	}

	return 0.f;
}

void PortfolioDebugOverlayEditorCVarAccess::SetFloat(const TCHAR* InName, float InValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		consoleVariable->Set(InValue, ECVF_SetByConsole);
	}
}

FString PortfolioDebugOverlayEditorCVarAccess::GetString(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		return consoleVariable->GetString();
	}

	return FString();
}

void PortfolioDebugOverlayEditorCVarAccess::SetString(const TCHAR* InName, const FString& InValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName))
	{
		consoleVariable->Set(*InValue, ECVF_SetByConsole);
	}
}

// ===== EventLog Filter Helpers =====

bool PortfolioDebugOverlayEditorCVarAccess::IsKnownEventLogFilter(const FString& InValue)
{
	return InValue.Equals(TEXT("All"), ESearchCase::IgnoreCase)
		|| InValue.Equals(TEXT("Execution"), ESearchCase::IgnoreCase)
		|| InValue.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)
		|| InValue.Equals(TEXT("AI"), ESearchCase::IgnoreCase)
		|| InValue.Equals(TEXT("Balance"), ESearchCase::IgnoreCase)
		|| InValue.Equals(TEXT("Death"), ESearchCase::IgnoreCase);
}

// ===== Availability =====

bool PortfolioDebugOverlayEditorCVarAccess::HasOverlayCVars()
{
	return FindCVar(DebugOverlayEnabledCVarName)
		&& FindCVar(DebugOverlayCollectCVarName)
		&& FindCVar(DebugOverlayEventLogFilterCVarName)
		&& FindCVar(DebugOverlayEventLogLimitCVarName)
		&& FindCVar(DebugOverlayHideNoiseEventsCVarName)
		&& FindCVar(DebugOverlayHideCollisionWindowEventsCVarName)
		&& FindCVar(DeathLifecycleAuditCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasTargetingDisplayCVars()
{
	return FindCVar(DebugOverlayTargetingEnabledCVarName)
		&& FindCVar(DebugOverlayTargetingDrawRangeSphereCVarName)
		&& FindCVar(DebugOverlayTargetingDrawSelectedTargetSphereCVarName)
		&& FindCVar(DebugOverlayTargetingDrawViewLineCVarName)
		&& FindCVar(DebugOverlayTargetingDrawDebugTextCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasMainPanelSectionCVars()
{
	return FindCVar(DebugOverlayPlayerPanelEnabledCVarName)
		&& FindCVar(DebugOverlayPlayerStatusEnabledCVarName)
		&& FindCVar(DebugOverlayPlayerTargetingEnabledCVarName)
		&& FindCVar(DebugOverlayPlayerLocomotionEnabledCVarName)
		&& FindCVar(DebugOverlayPlayerRecentExecutionEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyPanelEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyFocusEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyStatusEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyBalanceCollapseEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyCombatParticipationEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyDeathLifecycleEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyRecentExecutionEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyCurrentAIEnabledCVarName)
		&& FindCVar(DebugOverlayEnemyRecentAIEventEnabledCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasMovementDisplayCVars()
{
	return FindCVar(DebugOverlayMovementEnabledCVarName)
		&& FindCVar(DebugOverlayMovementDrawVelocityCVarName)
		&& FindCVar(DebugOverlayMovementDrawInputCVarName)
		&& FindCVar(DebugOverlayMovementDrawFacingCVarName)
		&& FindCVar(DebugOverlayMovementDrawDebugTextCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasBalanceDisplayCVars()
{
	return FindCVar(DebugOverlayBalanceEnabledCVarName)
		&& FindCVar(DebugOverlayBalanceDrawWorldTextCVarName)
		&& FindCVar(DebugOverlayBalanceDrawLifecycleBarCVarName)
		&& FindCVar(BalanceAuditCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasCombatParticipationDisplayCVars()
{
	return FindCVar(DebugOverlayCombatParticipationEnabledCVarName)
		&& FindCVar(DebugOverlayCombatParticipationDrawWorldTextCVarName)
		&& FindCVar(DebugOverlayCombatParticipationDrawWorldRingCVarName)
		&& FindCVar(DebugOverlayCombatParticipationDrawHitReactiveEvidenceAnchorCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasFocusCVars()
{
	return FindCVar(DebugOverlayNearestFocusRadiusCVarName)
		&& FindCVar(DebugOverlayFocusLiveSyncPlayerTargetCVarName);
}

FText PortfolioDebugOverlayEditorCVarAccess::GetAvailabilityText(const TCHAR* InName)
{
	return FindCVar(InName)
		? FText::GetEmpty()
		: LOCTEXT("UnavailableCVar", "Unavailable");
}

#undef LOCTEXT_NAMESPACE
