#pragma once

#include "CoreMinimal.h"

class IConsoleVariable;

namespace PortfolioDebugOverlayEditorCVarAccess
{
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

	FText GetAvailabilityText(const TCHAR* InName);
}
