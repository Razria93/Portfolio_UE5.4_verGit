#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "Type/CActionDataTypes.h"
#include "Type/CActionOrchestrationTypes.h"

class UAnimMontage;

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
	// Action Executor Diagnostic Hook
	static void RecordActionExecutorStartedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData);
	static void RecordActionExecutorStoppedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const TCHAR* InEvent);
	static void RecordActionMontagePlayedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, float InDuration);
	static void RecordActionExecutorRejectedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordActionMontageRejectedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordActionMontageIgnoredForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const UAnimMontage* InMontage, uint32 InSerial, uint32 InActiveSerial, const TCHAR* InReason);

public:
	// Debug Dump
	static void PrintActionExecutionContextDebug(const AActor* InOwnerActor, const FActionExecutionContext& InContext, const TCHAR* InEvent);
	static void PrintActionExecutorRuntimeDebug(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const UAnimMontage* InMontage, uint32 InSerial, const TCHAR* InEvent);
};
