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

	// ===== Targeting Display CVar Names =====

	static constexpr const TCHAR* DebugOverlayTargetingEnabledCVarName = TEXT("Portfolio.DebugOverlay.Targeting.Enabled");
	static constexpr const TCHAR* DebugOverlayTargetingDrawRangeSphereCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawRangeSphere");
	static constexpr const TCHAR* DebugOverlayTargetingDrawSelectedTargetSphereCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawSelectedTargetSphere");
	static constexpr const TCHAR* DebugOverlayTargetingDrawViewLineCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawViewLine");
	static constexpr const TCHAR* DebugOverlayTargetingDrawDebugTextCVarName = TEXT("Portfolio.DebugOverlay.Targeting.DrawDebugText");
	static constexpr const TCHAR* DebugOverlayTargetingShowOverlayDetailsCVarName = TEXT("Portfolio.DebugOverlay.Targeting.ShowOverlayDetails");

	// ===== Movement Display CVar Names =====

	static constexpr const TCHAR* DebugOverlayMovementEnabledCVarName = TEXT("Portfolio.DebugOverlay.Movement.Enabled");
	static constexpr const TCHAR* DebugOverlayMovementDrawVelocityCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawVelocity");
	static constexpr const TCHAR* DebugOverlayMovementDrawInputCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawInput");
	static constexpr const TCHAR* DebugOverlayMovementDrawFacingCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawFacing");
	static constexpr const TCHAR* DebugOverlayMovementDrawDebugTextCVarName = TEXT("Portfolio.DebugOverlay.Movement.DrawDebugText");
	static constexpr const TCHAR* DebugOverlayMovementShowOverlayDetailsCVarName = TEXT("Portfolio.DebugOverlay.Movement.ShowOverlayDetails");

	// ===== Combat Participation Display CVar Names =====

	static constexpr const TCHAR* DebugOverlayCombatParticipationEnabledCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.Enabled");
	static constexpr const TCHAR* DebugOverlayCombatParticipationDrawWorldTextCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldText");
	static constexpr const TCHAR* DebugOverlayCombatParticipationDrawWorldRingCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.DrawWorldRing");
	static constexpr const TCHAR* DebugOverlayCombatParticipationShowOverlayDetailsCVarName = TEXT("Portfolio.DebugOverlay.CombatParticipation.ShowOverlayDetails");

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

// ===== Targeting Display CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingEnabledCVarName() { return DebugOverlayTargetingEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawRangeSphereCVarName() { return DebugOverlayTargetingDrawRangeSphereCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawSelectedTargetSphereCVarName() { return DebugOverlayTargetingDrawSelectedTargetSphereCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawViewLineCVarName() { return DebugOverlayTargetingDrawViewLineCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingDrawDebugTextCVarName() { return DebugOverlayTargetingDrawDebugTextCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetTargetingShowOverlayDetailsCVarName() { return DebugOverlayTargetingShowOverlayDetailsCVarName; }
// ===== Movement Display CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementEnabledCVarName() { return DebugOverlayMovementEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawVelocityCVarName() { return DebugOverlayMovementDrawVelocityCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawInputCVarName() { return DebugOverlayMovementDrawInputCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawFacingCVarName() { return DebugOverlayMovementDrawFacingCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementDrawDebugTextCVarName() { return DebugOverlayMovementDrawDebugTextCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetMovementShowOverlayDetailsCVarName() { return DebugOverlayMovementShowOverlayDetailsCVarName; }

// ===== Combat Participation Display CVar Names =====

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationEnabledCVarName() { return DebugOverlayCombatParticipationEnabledCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationDrawWorldTextCVarName() { return DebugOverlayCombatParticipationDrawWorldTextCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationDrawWorldRingCVarName() { return DebugOverlayCombatParticipationDrawWorldRingCVarName; }
const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetCombatParticipationShowOverlayDetailsCVarName() { return DebugOverlayCombatParticipationShowOverlayDetailsCVarName; }

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
		&& FindCVar(DebugOverlayTargetingDrawDebugTextCVarName)
		&& FindCVar(DebugOverlayTargetingShowOverlayDetailsCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasMovementDisplayCVars()
{
	return FindCVar(DebugOverlayMovementEnabledCVarName)
		&& FindCVar(DebugOverlayMovementDrawVelocityCVarName)
		&& FindCVar(DebugOverlayMovementDrawInputCVarName)
		&& FindCVar(DebugOverlayMovementDrawFacingCVarName)
		&& FindCVar(DebugOverlayMovementDrawDebugTextCVarName)
		&& FindCVar(DebugOverlayMovementShowOverlayDetailsCVarName);
}

bool PortfolioDebugOverlayEditorCVarAccess::HasCombatParticipationDisplayCVars()
{
	return FindCVar(DebugOverlayCombatParticipationEnabledCVarName)
		&& FindCVar(DebugOverlayCombatParticipationDrawWorldTextCVarName)
		&& FindCVar(DebugOverlayCombatParticipationDrawWorldRingCVarName)
		&& FindCVar(DebugOverlayCombatParticipationShowOverlayDetailsCVarName);
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
