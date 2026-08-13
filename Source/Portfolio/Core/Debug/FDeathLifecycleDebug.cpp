#include "Core/Debug/FDeathLifecycleDebug.h"

#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FLog.h"

#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarDeathLifecycleAudit(
		TEXT("Portfolio.Debug.DeathLifecycleAudit"),
		1,
		TEXT("Print death lifecycle contract violation audit logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif

	FString FormatDeathLifecycleSummary(const AActor* InOwnerActor, const FString& InSummary)
	{
		const FString ownerName = GetNameSafe(InOwnerActor);
		return InSummary.IsEmpty()
			? FString::Printf(TEXT("Owner: %s"), *ownerName)
			: FString::Printf(TEXT("Owner: %s | %s"), *ownerName, *InSummary);
	}
}

// Gate

bool FDeathLifecycleDebug::ShouldAuditDeathLifecycle()
{
#if !UE_BUILD_SHIPPING
	return CVarDeathLifecycleAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Lifecycle Diagnostic Hook

void FDeathLifecycleDebug::RecordLifecycleEvent(const AActor* InOwnerActor, const TCHAR* InEvent, const FString& InSummary)
{
	if (!IsValid(InOwnerActor)) return;

	const FString ownerName = GetNameSafe(InOwnerActor);
	FDebugOverlaySnapshotStore::AddEvent(
		InOwnerActor,
		TEXT("Death"),
		InEvent ? InEvent : TEXT("Unknown"),
		ownerName,
		ownerName,
		FString(),
		FormatDeathLifecycleSummary(InOwnerActor, InSummary));
}

void FDeathLifecycleDebug::RecordContractViolationForAudit(const AActor* InOwnerActor, const TCHAR* InEvent, const FString& InSummary)
{
	RecordLifecycleEvent(InOwnerActor, InEvent, InSummary);

	if (!ShouldAuditDeathLifecycle()) return;

	FLog::Log(FString::Printf(
		TEXT("[DeathLifecycle|ContractViolation|%s] %s"),
		InEvent ? InEvent : TEXT("Unknown"),
		*FormatDeathLifecycleSummary(InOwnerActor, InSummary)));
}
