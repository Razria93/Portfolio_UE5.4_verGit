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
	const TCHAR* GetTargetingEnabledCVarName();
	const TCHAR* GetTargetingDrawRangeSphereCVarName();
	const TCHAR* GetTargetingDrawSelectedTargetSphereCVarName();
	const TCHAR* GetTargetingDrawViewLineCVarName();
	const TCHAR* GetTargetingDrawDebugTextCVarName();
	const TCHAR* GetTargetingShowOverlayDetailsCVarName();
	const TCHAR* GetFocusLiveSyncPlayerTargetCVarName();

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
	bool HasOverlayCVars();
	bool HasTargetingDisplayCVars();
	bool HasFocusCVars();
	FText GetAvailabilityText(const TCHAR* InName);
}
