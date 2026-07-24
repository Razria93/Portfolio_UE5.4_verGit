#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionTypes.h"
#include "Type/CReactionDataTypes.h"
#include "Type/CReactionOrchestrationTypes.h"

class UAnimMontage;

class PORTFOLIO_API FReactionComponentDebug
{
public:
	// Gate
	static bool ShouldAuditReactionComponent();
	static bool ShouldPrintReactionComponentDebug();

public:
	// Data / Executor Diagnostic Hook
	static void RecordReactionDataDuplicateForAudit(const AActor* InOwnerActor, const FReactionData& InData, bool bInRebuildAll);
	static void RecordReactionDataResolvedForAudit(const AActor* InOwnerActor, const FReactionDataKey& InRequestKey, const FReactionData& InResolvedData, int32 InCandidateIndex);
	static void RecordReactionDataResolveFailedForAudit(const AActor* InOwnerActor, const FReactionDataKey& InDataKey, const TCHAR* InReason);
	static void RecordReactionExecutorResolveFailedForAudit(const AActor* InOwnerActor, const FReactionData& InData, const TCHAR* InReason);
	static void RecordReactionExecutorMapBuildFailedForAudit(const AActor* InOwnerActor, const FReactionData& InData, const TCHAR* InReason);

public:
	// Execution Diagnostic Hook
	static void RecordReactionDecisionAppliedForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent);
	static void RecordReactionDecisionRejectedForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordReactionRuntimeAcceptedForAudit(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent);
	static void RecordReactionRuntimeRejectedForAudit(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent, const TCHAR* InReason);

public:
	// Notify Diagnostic Hook
	static void RecordReactionNotifyIgnoredForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const TCHAR* InEvent, FName InKey, const TCHAR* InReason);
	static void RecordReactionNotifyCommandIgnoredForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, EReactionNotifyCommand InCommand, const TCHAR* InReason);

public:
	// Reaction Executor Diagnostic Hook
	static void RecordReactionExecutorStartedForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const FReactionData& InData);
	static void RecordReactionExecutorStoppedForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const FReactionData& InData, const TCHAR* InEvent);
	static void RecordReactionMontagePlayedForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const FReactionData& InData, float InDuration);
	static void RecordReactionExecutorRejectedForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const FReactionData& InData, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordReactionMontageRejectedForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const FReactionData& InData, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordReactionMontageIgnoredForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const UAnimMontage* InMontage, uint32 InSerial, uint32 InActiveSerial, const TCHAR* InReason);

public:
	// Debug Dump
	static void PrintReactionExecutionContextDebug(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent);
	static void PrintReactionExecutorRuntimeDebug(const AActor* InOwnerActor, const UObject* InReactionExecutor, const FReactionData& InData, const UAnimMontage* InMontage, uint32 InSerial, const TCHAR* InEvent);
};
