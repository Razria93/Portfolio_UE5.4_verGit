#include "FPortfolioDebugOverlayEditorCVarAccess.h"

#include "HAL/IConsoleManager.h"

#define LOCTEXT_NAMESPACE "FPortfolioDebugOverlayEditorCVarAccess"

IConsoleVariable* PortfolioDebugOverlayEditorCVarAccess::FindCVar(const TCHAR* InName)
{
	return InName ? IConsoleManager::Get().FindConsoleVariable(InName) : nullptr;
}

bool PortfolioDebugOverlayEditorCVarAccess::GetBool(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName)) return consoleVariable->GetInt() != 0;
	return false;
}

void PortfolioDebugOverlayEditorCVarAccess::SetBool(const TCHAR* InName, const bool bInValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName)) consoleVariable->Set(bInValue ? 1 : 0, ECVF_SetByCode);
}

int32 PortfolioDebugOverlayEditorCVarAccess::GetInt(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName)) return consoleVariable->GetInt();
	return 0;
}

void PortfolioDebugOverlayEditorCVarAccess::SetInt(const TCHAR* InName, const int32 InValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName)) consoleVariable->Set(InValue, ECVF_SetByCode);
}

float PortfolioDebugOverlayEditorCVarAccess::GetFloat(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName)) return consoleVariable->GetFloat();
	return 0.f;
}

void PortfolioDebugOverlayEditorCVarAccess::SetFloat(const TCHAR* InName, const float InValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName)) consoleVariable->Set(InValue, ECVF_SetByCode);
}

FString PortfolioDebugOverlayEditorCVarAccess::GetString(const TCHAR* InName)
{
	if (const IConsoleVariable* consoleVariable = FindCVar(InName)) return consoleVariable->GetString();
	return FString();
}

void PortfolioDebugOverlayEditorCVarAccess::SetString(const TCHAR* InName, const FString& InValue)
{
	if (IConsoleVariable* consoleVariable = FindCVar(InName)) consoleVariable->Set(*InValue, ECVF_SetByCode);
}

FText PortfolioDebugOverlayEditorCVarAccess::GetAvailabilityText(const TCHAR* InName)
{
	return FindCVar(InName)
		? LOCTEXT("AvailableCVar", "")
		: LOCTEXT("UnavailableCVar", "Unavailable");
}

#undef LOCTEXT_NAMESPACE
