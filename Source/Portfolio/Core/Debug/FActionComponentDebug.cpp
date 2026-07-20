#include "Core/Debug/FActionComponentDebug.h"
#include "Core/Debug/FLog.h"

#include "Action/CAction.h"
#include "Animation/AnimMontage.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarActionComponentAudit(
		TEXT("Portfolio.Debug.ActionComponentAudit"),
		0,
		TEXT("Print action component data/executor/notify diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarActionComponentDump(
		TEXT("Portfolio.Debug.ActionComponentDump"),
		0,
		TEXT("Print action component debug dump logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatActionComponentDataKey(const FActionDataKey& InDataKey)
	{
		return FString::Printf(
			TEXT("ActionType=%s | ActionIndex=%d"),
			*UEnum::GetValueAsString(InDataKey.ActionType),
			InDataKey.ActionIndex);
	}

	FString FormatActionComponentData(const FActionData& InData)
	{
		return FString::Printf(
			TEXT("%s | ExecutorKey=%s | Montage=%s | Priority=%d | CanMove=%s"),
			*FormatActionComponentDataKey(InData.ActionDataKey),
			*GetNameSafe(InData.ActionExecutorKey.Get()),
			*GetNameSafe(InData.Montage),
			InData.Priority,
			InData.bCanMove ? TEXT("true") : TEXT("false"));
	}

	FString FormatActionComponentExecutionResult(const FActionExecutionResult& InResult)
	{
		return FString::Printf(
			TEXT("Decision=%s | Relationship=%s | ApplyMode=%s | RejectReason=%s | %s"),
			*UEnum::GetValueAsString(InResult.Decision),
			*UEnum::GetValueAsString(InResult.Relationship),
			*UEnum::GetValueAsString(InResult.ApplyMode),
			*UEnum::GetValueAsString(InResult.RejectReason),
			*FormatActionComponentDataKey(InResult.ResolvedContext.ActionDataKey));
	}
}

// Gate

bool FActionComponentDebug::ShouldAuditActionComponent()
{
#if !UE_BUILD_SHIPPING
	return CVarActionComponentAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FActionComponentDebug::ShouldPrintActionComponentDebug()
{
#if !UE_BUILD_SHIPPING
	return CVarActionComponentDump.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Data / Executor Diagnostic Hook

void FActionComponentDebug::RecordActionDataDuplicateForAudit(const AActor* InOwnerActor, const FActionData& InData, bool bInRebuildAll)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|DataDuplicate] Owner=%s | RebuildAll=%s | %s"),
		*GetNameSafe(InOwnerActor),
		bInRebuildAll ? TEXT("true") : TEXT("false"),
		*FormatActionComponentData(InData)));
}

void FActionComponentDebug::RecordActionDataResolveFailedForAudit(const AActor* InOwnerActor, const FActionDataKey& InDataKey, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|DataResolveFailed] Reason=%s | Owner=%s | %s"),
		InReason ? InReason : TEXT("Unknown"),
		*GetNameSafe(InOwnerActor),
		*FormatActionComponentDataKey(InDataKey)));
}

void FActionComponentDebug::RecordActionExecutorResolveFailedForAudit(const AActor* InOwnerActor, const FActionData& InData, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|ExecutorResolveFailed] Reason=%s | Owner=%s | %s"),
		InReason ? InReason : TEXT("Unknown"),
		*GetNameSafe(InOwnerActor),
		*FormatActionComponentData(InData)));
}

void FActionComponentDebug::RecordActionExecutorMapBuildFailedForAudit(const AActor* InOwnerActor, const FActionData& InData, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|ExecutorMapBuildFailed] Reason=%s | Owner=%s | %s"),
		InReason ? InReason : TEXT("Unknown"),
		*GetNameSafe(InOwnerActor),
		*FormatActionComponentData(InData)));
}

// Execution Diagnostic Hook

void FActionComponentDebug::RecordActionDecisionAppliedForAudit(const AActor* InOwnerActor, const FActionExecutionResult& InResult, const TCHAR* InEvent)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|%sApplied] Owner=%s | %s"),
		InEvent ? InEvent : TEXT("Decision"),
		*GetNameSafe(InOwnerActor),
		*FormatActionComponentExecutionResult(InResult)));
}

void FActionComponentDebug::RecordActionDecisionRejectedForAudit(const AActor* InOwnerActor, const FActionExecutionResult& InResult, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|%sRejected] Reason=%s | Owner=%s | %s"),
		InEvent ? InEvent : TEXT("Decision"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*FormatActionComponentExecutionResult(InResult)));
}

void FActionComponentDebug::RecordActionRuntimeRejectedForAudit(const AActor* InOwnerActor, const FActionExecutionContext& InContext, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|%sRejected] Reason=%s | Owner=%s | %s | Executor=%s"),
		InEvent ? InEvent : TEXT("Runtime"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*FormatActionComponentData(InContext.ActionData),
		*GetNameSafe(InContext.ActionExecutor)));
}

// Notify Diagnostic Hook

void FActionComponentDebug::RecordActionCombatSignalCueForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, FName InCueTag, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|CombatSignalCue%s] Reason=%s | Owner=%s | Executor=%s | CueTag=%s"),
		InEvent ? InEvent : TEXT(""),
		InReason ? InReason : TEXT("None"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*InCueTag.ToString()));
}

void FActionComponentDebug::RecordActionNotifyIgnoredForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const TCHAR* InEvent, FName InKey, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|%sIgnored] Reason=%s | Owner=%s | Executor=%s | Key=%s"),
		InEvent ? InEvent : TEXT("Notify"),
		InReason ? InReason : TEXT("Ignored"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*InKey.ToString()));
}

void FActionComponentDebug::RecordActionNotifyCommandIgnoredForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, EActionNotifyCommand InCommand, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|NotifyCommandIgnored] Reason=%s | Owner=%s | Executor=%s | Command=%s"),
		InReason ? InReason : TEXT("Ignored"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*UEnum::GetValueAsString(InCommand)));
}

// Debug Dump

void FActionComponentDebug::PrintActionExecutionContextDebug(const AActor* InOwnerActor, const FActionExecutionContext& InContext, const TCHAR* InEvent)
{
	if (!ShouldPrintActionComponentDebug()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Component|%sContextDump] Owner=%s | %s | Executor=%s"),
		InEvent ? InEvent : TEXT("Execution"),
		*GetNameSafe(InOwnerActor),
		*FormatActionComponentData(InContext.ActionData),
		*GetNameSafe(InContext.ActionExecutor)));
}
