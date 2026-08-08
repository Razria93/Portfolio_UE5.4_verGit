#pragma once

#include "CoreMinimal.h"

class IConsoleVariable;

namespace PortfolioDebugOverlayEditorCVarAccess
{
	const TCHAR* GetEnabledCVarName();
	const TCHAR* GetCollectCVarName();
	const TCHAR* GetEventLogFilterCVarName();
	const TCHAR* GetEventLogLimitCVarName();
	const TCHAR* GetHideNoiseEventsCVarName();
	const TCHAR* GetHideCollisionWindowEventsCVarName();
	const TCHAR* GetNearestFocusRadiusCVarName();

	IConsoleVariable* FindCVar(const TCHAR* InName);

	bool GetBool(const TCHAR* InName);
	void SetBool(const TCHAR* InName, bool bInValue);

	int32 GetInt(const TCHAR* InName);
	void SetInt(const TCHAR* InName, int32 InValue);

	float GetFloat(const TCHAR* InName);
	void SetFloat(const TCHAR* InName, float InValue);

	FString GetString(const TCHAR* InName);
	void SetString(const TCHAR* InName, const FString& InValue);

	bool IsKnownEventLogFilter(const FString& InValue);
	bool HasAllRequiredCVars();
	FText GetAvailabilityText(const TCHAR* InName);
}
