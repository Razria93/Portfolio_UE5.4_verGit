#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionOrchestrationStructure.h"
#include "Type/CWeaponStructure.h"

class PORTFOLIO_API FReactionComponentDebug
{
public:
	// Gate
	static bool ShouldAuditReactionComponent();
	static bool ShouldPrintReactionComponentDebug();

public:
	// Data / Executor Diagnostic Hook
	static void RecordReactionDataDuplicateForAudit(const AActor* InOwnerActor, const FReactionData& InData, bool bInRebuildAll);
	static void RecordReactionDataResolveFailedForAudit(const AActor* InOwnerActor, const FReactionDataKey& InDataKey, const TCHAR* InReason);
	static void RecordReactionDataResolvedForAudit(const AActor* InOwnerActor, const FReactionDataKey& InRequestKey, const FReactionData& InResolvedData, int32 InCandidateIndex);
	static void RecordReactionExecutorResolveFailedForAudit(const AActor* InOwnerActor, const FReactionData& InData, const TCHAR* InReason);
	static void RecordReactionExecutorMapBuildFailedForAudit(const AActor* InOwnerActor, const FReactionData& InData, const TCHAR* InReason);

public:
	// Execution Diagnostic Hook
	static void RecordReactionDecisionAppliedForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent);
	static void RecordReactionDecisionRejectedForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordReactionRuntimeRejectedForAudit(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordReactionRuntimeAcceptedForAudit(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent);

public:
	// Notify Diagnostic Hook
	static void RecordReactionNotifyIgnoredForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const TCHAR* InEvent, FName InKey, const TCHAR* InReason);
	static void RecordReactionNotifyCommandIgnoredForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, EReactionNotifyCommand InCommand, const TCHAR* InReason);

public:
	// Debug Dump
	static void PrintReactionExecutionContextDebug(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent);
};
