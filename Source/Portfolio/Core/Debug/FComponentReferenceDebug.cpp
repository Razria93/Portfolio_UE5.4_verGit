#include "Core/Debug/FComponentReferenceDebug.h"
#include "Core/Debug/FLog.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarComponentReferenceAudit(
		TEXT("Portfolio.Debug.ComponentReferenceAudit"),
		0,
		TEXT("Print component reference recovery diagnostic hook logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif
}

// Gate

bool FComponentReferenceDebug::ShouldAuditComponentReference()
{
#if !UE_BUILD_SHIPPING
	return CVarComponentReferenceAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Component Reference Diagnostic Hook

void FComponentReferenceDebug::RecordComponentReferenceRecoveredForAudit(const AActor* InOwnerActor, const UClass* InComponentClass, const UObject* InResolvedComponent)
{
	if (!ShouldAuditComponentReference()) return;

	FLog::Log(FString::Printf(
		TEXT("[ComponentReference|Recovery|Recovered] Owner=%s | Component=%s | Resolved=%s"),
		*GetNameSafe(InOwnerActor),
		*GetNameSafe(InComponentClass),
		*GetNameSafe(InResolvedComponent)));
}
