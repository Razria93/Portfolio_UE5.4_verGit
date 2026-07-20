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

// Action Executor Diagnostic Hook

void FActionComponentDebug::RecordActionExecutorStartedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Executor|Started] Owner=%s | Executor=%s | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*FormatActionComponentData(InData)));
}

void FActionComponentDebug::RecordActionExecutorStoppedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const TCHAR* InEvent)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Executor|%s] Owner=%s | Executor=%s | %s"),
		InEvent ? InEvent : TEXT("Stopped"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*FormatActionComponentData(InData)));
}

void FActionComponentDebug::RecordActionMontagePlayedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, float InDuration)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Executor|MontagePlayed] Owner=%s | Executor=%s | Duration=%.3f | %s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		InDuration,
		*FormatActionComponentData(InData)));
}

void FActionComponentDebug::RecordActionExecutorRejectedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Executor|%sRejected] Reason=%s | Owner=%s | Executor=%s | %s"),
		InEvent ? InEvent : TEXT("Lifecycle"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*FormatActionComponentData(InData)));
}

void FActionComponentDebug::RecordActionMontageRejectedForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Executor|%sRejected] Reason=%s | Owner=%s | Executor=%s | %s"),
		InEvent ? InEvent : TEXT("Montage"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*FormatActionComponentData(InData)));
}

void FActionComponentDebug::RecordActionMontageIgnoredForAudit(const AActor* InOwnerActor, const UObject* InActionExecutor, const UAnimMontage* InMontage, uint32 InSerial, uint32 InActiveSerial, const TCHAR* InReason)
{
	if (!ShouldAuditActionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Executor|MontageIgnored] Reason=%s | Owner=%s | Executor=%s | Montage=%s | Serial=%u | ActiveSerial=%u"),
		InReason ? InReason : TEXT("Ignored"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*GetNameSafe(InMontage),
		InSerial,
		InActiveSerial));
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

void FActionComponentDebug::PrintActionExecutorRuntimeDebug(const AActor* InOwnerActor, const UObject* InActionExecutor, const FActionData& InData, const UAnimMontage* InMontage, uint32 InSerial, const TCHAR* InEvent)
{
	if (!ShouldPrintActionComponentDebug()) return;

	FLog::Log(FString::Printf(
		TEXT("[Action|Executor|%sRuntimeDump] Owner=%s | Executor=%s | Montage=%s | Serial=%u | %s"),
		InEvent ? InEvent : TEXT("Action"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InActionExecutor),
		*GetNameSafe(InMontage),
		InSerial,
		*FormatActionComponentData(InData)));
}
