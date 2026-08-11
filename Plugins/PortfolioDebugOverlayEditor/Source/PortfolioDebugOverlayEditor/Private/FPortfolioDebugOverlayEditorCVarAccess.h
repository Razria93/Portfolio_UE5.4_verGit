#pragma once

#include "CoreMinimal.h"

class IConsoleVariable;

namespace PortfolioDebugOverlayEditorCVarAccess
{
	// ===== Overlay CVars =====

	const TCHAR* GetEnabledCVarName();
	const TCHAR* GetCollectCVarName();
	const TCHAR* GetEventLogFilterCVarName();
	const TCHAR* GetEventLogLimitCVarName();
	const TCHAR* GetHideNoiseEventsCVarName();
	const TCHAR* GetHideCollisionWindowEventsCVarName();
	const TCHAR* GetDeathLifecycleAuditCVarName();
	const TCHAR* GetNearestFocusRadiusCVarName();

	// ===== Targeting Display CVars =====

	const TCHAR* GetTargetingEnabledCVarName();
	const TCHAR* GetTargetingDrawRangeSphereCVarName();
	const TCHAR* GetTargetingDrawSelectedTargetSphereCVarName();
	const TCHAR* GetTargetingDrawViewLineCVarName();
	const TCHAR* GetTargetingDrawDebugTextCVarName();
	const TCHAR* GetTargetingShowOverlayDetailsCVarName();

	// ===== Movement Display CVars =====

	const TCHAR* GetMovementEnabledCVarName();
	const TCHAR* GetMovementDrawVelocityCVarName();
	const TCHAR* GetMovementDrawInputCVarName();
	const TCHAR* GetMovementDrawFacingCVarName();
	const TCHAR* GetMovementDrawDebugTextCVarName();
	const TCHAR* GetMovementShowOverlayDetailsCVarName();

	// ===== Focus CVars =====

	const TCHAR* GetFocusLiveSyncPlayerTargetCVarName();

	// ===== Generic CVar Access =====

	IConsoleVariable* FindCVar(const TCHAR* InName);

	bool GetBool(const TCHAR* InName);
	void SetBool(const TCHAR* InName, bool bInValue);

	int32 GetInt(const TCHAR* InName);
	void SetInt(const TCHAR* InName, int32 InValue);

	float GetFloat(const TCHAR* InName);
	void SetFloat(const TCHAR* InName, float InValue);

	FString GetString(const TCHAR* InName);
	void SetString(const TCHAR* InName, const FString& InValue);

	// ===== CVar Availability =====

	bool HasOverlayCVars();
	bool HasTargetingDisplayCVars();
	bool HasMovementDisplayCVars();
	bool HasFocusCVars();
	FText GetAvailabilityText(const TCHAR* InName);

	// ===== EventLog Filter =====

	bool IsKnownEventLogFilter(const FString& InValue);
}
