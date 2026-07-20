#include "Core/Debug/FReactionComponentDebug.h"
#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"
#include "Reaction/CReaction.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarReactionComponentAudit(
		TEXT("Portfolio.Debug.ReactionComponentAudit"),
		0,
		TEXT("Print reaction component data/executor/notify/runtime diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarReactionComponentDump(
		TEXT("Portfolio.Debug.ReactionComponentDump"),
		0,
		TEXT("Print reaction component debug dump logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatReactionComponentDamageSpecKey(const FDamageSpecKey& InSpecKey)
	{
		return FString::Printf(
			TEXT("WeaponType=%s | ActionType=%s | ActionIndex=%d"),
			*UEnum::GetValueAsString(InSpecKey.WeaponType),
			*UEnum::GetValueAsString(InSpecKey.ActionType),
			InSpecKey.ActionIndex);
	}

	FString FormatReactionComponentDataKey(const FReactionDataKey& InDataKey)
	{
		return FString::Printf(
			TEXT("ReactionType=%s | %s"),
			*UEnum::GetValueAsString(InDataKey.ReactionType),
			*FormatReactionComponentDamageSpecKey(InDataKey.DamageSpecKey));
	}

	FString FormatReactionComponentData(const FReactionData& InData)
	{
		return FString::Printf(
			TEXT("%s | ExecutorKey=%s | Montage=%s | Priority=%d | CanMove=%s"),
			*FormatReactionComponentDataKey(InData.ReactionDataKey),
			*GetNameSafe(InData.ReactionExecutorKey.Get()),
			*GetNameSafe(InData.Montage),
			InData.Priority,
			InData.bCanMove ? TEXT("true") : TEXT("false"));
	}

	FString FormatReactionComponentExecutionResult(const FReactionExecutionResult& InResult)
	{
		return FString::Printf(
			TEXT("Decision=%s | Relationship=%s | ApplyMode=%s | RejectReason=%s | %s"),
			*UEnum::GetValueAsString(InResult.Decision),
			*UEnum::GetValueAsString(InResult.Relationship),
			*UEnum::GetValueAsString(InResult.ApplyMode),
			*UEnum::GetValueAsString(InResult.RejectReason),
			*FormatReactionComponentDataKey(InResult.ResolvedContext.ReactionDataKey));
	}
}

// Gate

bool FReactionComponentDebug::ShouldAuditReactionComponent()
{
#if !UE_BUILD_SHIPPING
	return CVarReactionComponentAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FReactionComponentDebug::ShouldPrintReactionComponentDebug()
{
#if !UE_BUILD_SHIPPING
	return CVarReactionComponentDump.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Data / Executor Diagnostic Hook

void FReactionComponentDebug::RecordReactionDataDuplicateForAudit(const AActor* InOwnerActor, const FReactionData& InData, bool bInRebuildAll)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|DataDuplicate] Owner=%s | RebuildAll=%s | %s"),
		*GetNameSafe(InOwnerActor),
		bInRebuildAll ? TEXT("true") : TEXT("false"),
		*FormatReactionComponentData(InData)));
}

void FReactionComponentDebug::RecordReactionDataResolveFailedForAudit(const AActor* InOwnerActor, const FReactionDataKey& InDataKey, const TCHAR* InReason)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|DataResolveFailed] Reason=%s | Owner=%s | %s"),
		InReason ? InReason : TEXT("Unknown"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentDataKey(InDataKey)));
}

void FReactionComponentDebug::RecordReactionDataResolvedForAudit(const AActor* InOwnerActor, const FReactionDataKey& InRequestKey, const FReactionData& InResolvedData, int32 InCandidateIndex)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|DataResolved] Owner=%s | CandidateIndex=%d | Request=%s | Resolved=%s"),
		*GetNameSafe(InOwnerActor),
		InCandidateIndex,
		*FormatReactionComponentDataKey(InRequestKey),
		*FormatReactionComponentData(InResolvedData)));
}

void FReactionComponentDebug::RecordReactionExecutorResolveFailedForAudit(const AActor* InOwnerActor, const FReactionData& InData, const TCHAR* InReason)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|ExecutorResolveFailed] Reason=%s | Owner=%s | %s"),
		InReason ? InReason : TEXT("Unknown"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentData(InData)));
}

void FReactionComponentDebug::RecordReactionExecutorMapBuildFailedForAudit(const AActor* InOwnerActor, const FReactionData& InData, const TCHAR* InReason)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|ExecutorMapBuildFailed] Reason=%s | Owner=%s | %s"),
		InReason ? InReason : TEXT("Unknown"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentData(InData)));
}

// Execution Diagnostic Hook

void FReactionComponentDebug::RecordReactionDecisionAppliedForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|%sApplied] Owner=%s | %s"),
		InEvent ? InEvent : TEXT("Decision"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentExecutionResult(InResult)));
}

void FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(const AActor* InOwnerActor, const FReactionExecutionResult& InResult, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|%sRejected] Reason=%s | Owner=%s | %s"),
		InEvent ? InEvent : TEXT("Decision"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentExecutionResult(InResult)));
}

void FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent, const TCHAR* InReason)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|%sRejected] Reason=%s | Owner=%s | %s | Executor=%s"),
		InEvent ? InEvent : TEXT("Runtime"),
		InReason ? InReason : TEXT("Rejected"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentDataKey(InContext.ReactionDataKey),
		*GetNameSafe(InContext.ReactionExecutor)));
}

void FReactionComponentDebug::RecordReactionRuntimeAcceptedForAudit(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|%sAccepted] Owner=%s | %s | Executor=%s"),
		InEvent ? InEvent : TEXT("Runtime"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentDataKey(InContext.ReactionDataKey),
		*GetNameSafe(InContext.ReactionExecutor)));
}

// Notify Diagnostic Hook

void FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, const TCHAR* InEvent, FName InKey, const TCHAR* InReason)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|%sIgnored] Reason=%s | Owner=%s | Executor=%s | Key=%s"),
		InEvent ? InEvent : TEXT("Notify"),
		InReason ? InReason : TEXT("Ignored"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InReactionExecutor),
		*InKey.ToString()));
}

void FReactionComponentDebug::RecordReactionNotifyCommandIgnoredForAudit(const AActor* InOwnerActor, const UObject* InReactionExecutor, EReactionNotifyCommand InCommand, const TCHAR* InReason)
{
	if (!ShouldAuditReactionComponent()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|NotifyCommandIgnored] Reason=%s | Owner=%s | Executor=%s | Command=%s"),
		InReason ? InReason : TEXT("Ignored"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InReactionExecutor),
		*UEnum::GetValueAsString(InCommand)));
}

// Debug Dump

void FReactionComponentDebug::PrintReactionExecutionContextDebug(const AActor* InOwnerActor, const FReactionExecutionContext& InContext, const TCHAR* InEvent)
{
	if (!ShouldPrintReactionComponentDebug()) return;

	FLog::Log(FString::Printf(
		TEXT("[Reaction|Component|%sContextDump] Owner=%s | %s | Executor=%s"),
		InEvent ? InEvent : TEXT("Execution"),
		*GetNameSafe(InOwnerActor),
		*FormatReactionComponentData(InContext.ReactionData),
		*GetNameSafe(InContext.ReactionExecutor)));
}
