#pragma once

#include "CoreMinimal.h"
#include "Type/CActionOrchestrationStructure.h"
#include "Type/CWeaponStructure.h"

class PORTFOLIO_API FActionComponentDebug
{
public:
	// Gate
	static bool ShouldAuditActionComponent();
	static bool ShouldPrintActionComponentDebug();

public:
	// Data / Executor Diagnostic Hook
	static void RecordActionDataDuplicateForAudit(const AActor* InOwnerActor, const FActionData& InData, bool bInRebuildAll);
	static void RecordActionDataResolveFailedForAudit(const AActor* InOwnerActor, const FActionDataKey& InDataKey, const TCHAR* InReason);
	static void RecordActionExecutorResolveFailedForAudit(const AActor* InOwnerActor, const FActionData& InData, const TCHAR* InReason);
	static void RecordActionExecutorMapBuildFailedForAudit(const AActor* InOwnerActor, const FActionData& InData, const TCHAR* InReason);

public:
	// Execution Diagnostic Hook
	static void RecordActionDecisionAppliedForAudit(const AActor* InOwnerActor, const FActionExecutionResult& InResult, const TCHAR* InEvent);
	static void RecordActionDecisionRejectedForAudit(const AActor* InOwnerActor, const FActionExecutionResult& InResult, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordActionRuntimeRejectedForAudit(const AActor* InOwnerActor, const FActionExecutionContext& InContext, const TCHAR* InEvent, const TCHAR* InReason);

public:
	// Notify Diagnostic Hook
	static void RecordActionCombatSignalCueForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, FName InCueTag, const TCHAR* InEvent, const TCHAR* InReason = nullptr);
	static void RecordActionNotifyIgnoredForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const TCHAR* InEvent, FName InKey, const TCHAR* InReason);
	static void RecordActionNotifyCommandIgnoredForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, EActionNotifyCommand InCommand, const TCHAR* InReason);

public:
	// Debug Dump
	static void PrintActionExecutionContextDebug(const AActor* InOwnerActor, const FActionExecutionContext& InContext, const TCHAR* InEvent);
};
