#include "FPortfolioDebugOverlayEditorCVarAccess.h"

#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "FPortfolioDebugOverlayEditorCVarAccess"

namespace
{
	// ===== Constants =====

	static constexpr const TCHAR* DebugOverlayEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enabled");
	static constexpr const TCHAR* DebugOverlayCollectCVarName = TEXT("Portfolio.DebugOverlay.Collect");
	static constexpr const TCHAR* DebugOverlayEventLogFilterCVarName = TEXT("Portfolio.DebugOverlay.EventLogFilter");
	static constexpr const TCHAR* DebugOverlayEventLogLimitCVarName = TEXT("Portfolio.DebugOverlay.EventLogLimit");
	static constexpr const TCHAR* DebugOverlayHideNoiseEventsCVarName = TEXT("Portfolio.DebugOverlay.HideNoiseEvents");
	static constexpr const TCHAR* DebugOverlayHideCollisionWindowEventsCVarName = TEXT("Portfolio.DebugOverlay.HideCollisionWindowEvents");
	static constexpr const TCHAR* DebugOverlayNearestFocusRadiusCVarName = TEXT("Portfolio.DebugOverlay.NearestFocusRadius");
}

// ===== CVar Names =====

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

const TCHAR* PortfolioDebugOverlayEditorCVarAccess::GetNearestFocusRadiusCVarName()
{
	return DebugOverlayNearestFocusRadiusCVarName;
}

// ===== CVar Access =====

IConsoleVariable* PortfolioDebugOverlayEditorCVarAccess::FindCVar(const TCHAR* InName)
{
	return IConsoleManager::Get().FindConsoleVariable(InName);
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
		|| InValue.Equals(TEXT("AI"), ESearchCase::IgnoreCase);
}

// ===== Availability =====

bool PortfolioDebugOverlayEditorCVarAccess::HasAllRequiredCVars()
{
	return FindCVar(DebugOverlayEnabledCVarName)
		&& FindCVar(DebugOverlayCollectCVarName)
		&& FindCVar(DebugOverlayEventLogFilterCVarName)
		&& FindCVar(DebugOverlayEventLogLimitCVarName)
		&& FindCVar(DebugOverlayNearestFocusRadiusCVarName)
		&& FindCVar(DebugOverlayHideNoiseEventsCVarName)
		&& FindCVar(DebugOverlayHideCollisionWindowEventsCVarName);
}

FText PortfolioDebugOverlayEditorCVarAccess::GetAvailabilityText(const TCHAR* InName)
{
	return FindCVar(InName)
		? FText::GetEmpty()
		: LOCTEXT("UnavailableCVar", "Unavailable");
}

#undef LOCTEXT_NAMESPACE
