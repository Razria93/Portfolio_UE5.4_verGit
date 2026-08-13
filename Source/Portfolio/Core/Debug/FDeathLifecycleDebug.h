#pragma once

#include "CoreMinimal.h"

class PORTFOLIO_API FDeathLifecycleDebug
{
public:
	// Gate
	static bool ShouldAuditDeathLifecycle();

public:
	// Lifecycle Diagnostic Hook
	static void RecordLifecycleEvent(const AActor* InOwnerActor, const TCHAR* InEvent, const FString& InSummary = FString());
	static void RecordContractViolationForAudit(const AActor* InOwnerActor, const TCHAR* InEvent, const FString& InSummary = FString());
};
