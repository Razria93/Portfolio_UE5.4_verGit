#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"
#include "GameFramework/Actor.h"

#if !UE_BUILD_SHIPPING
namespace
{
	constexpr int32 MaxMotionActors = 64;

	template<typename T>
	void PrepareActorEntry(TMap<TWeakObjectPtr<AActor>, T>& InMap, const TWeakObjectPtr<AActor>& InKey)
	{
		for (auto it = InMap.CreateIterator(); it; ++it)
			if (!it.Key().IsValid()) it.RemoveCurrent();
		if (!InMap.Contains(InKey) && InMap.Num() >= MaxMotionActors)
			InMap.Remove(InMap.CreateConstIterator().Key());
	}
}
#endif

// Diagnostic Records
void FDebugOverlaySnapshotStore::RecordKnockbackDiagnostic(const AActor* InActor, const FCombatKnockbackDebugRecord& InRecord)
{
#if !UE_BUILD_SHIPPING
	if (!IsValid(InActor)) return;
	auto* store = StoreLifecycle::FindOrAddStore(InActor);
	if (!store) return;
	const TWeakObjectPtr<AActor> key(const_cast<AActor*>(InActor));
	PrepareActorEntry(store->KnockbackByActor, key);
	auto& history = store->KnockbackByActor.FindOrAdd(key);
	if (InRecord.Generation != 0 && InRecord.Generation < history.LatestGeneration) return;
	history.LatestGeneration = FMath::Max(history.LatestGeneration, InRecord.Generation);
	history.Last = InRecord;
	if (InRecord.bHasGeometry) history.Motion = InRecord;
#endif
}

void FDebugOverlaySnapshotStore::RecordAttackFacingDiagnostic(const AActor* InActor, const FActionFacingDebugRecord& InRecord)
{
#if !UE_BUILD_SHIPPING
	if (!IsValid(InActor)) return;
	auto* store = StoreLifecycle::FindOrAddStore(InActor);
	if (!store) return;
	const TWeakObjectPtr<AActor> key(const_cast<AActor*>(InActor));
	PrepareActorEntry(store->AttackFacingByActor, key);
	auto& last = store->AttackFacingByActor.FindOrAdd(key);
	if (InRecord.Generation < last.Generation) return;
	last = InRecord;
#endif
}

// Diagnostic Queries
bool FDebugOverlaySnapshotStore::TryGetKnockbackDiagnostic(const AActor* InActor, FCombatKnockbackDebugHistory& OutHistory)
{
	OutHistory = {};
#if !UE_BUILD_SHIPPING
	if (!IsValid(InActor)) return false;
	const auto* store = StoreLifecycle::FindStore(InActor);
	const auto* entry = store ? store->KnockbackByActor.Find(TWeakObjectPtr<AActor>(const_cast<AActor*>(InActor))) : nullptr;
	if (entry) { OutHistory = *entry; return true; }
#endif
	return false;
}

bool FDebugOverlaySnapshotStore::TryGetAttackFacingDiagnostic(const AActor* InActor, FActionFacingDebugRecord& OutRecord)
{
	OutRecord = {};
#if !UE_BUILD_SHIPPING
	if (!IsValid(InActor)) return false;
	const auto* store = StoreLifecycle::FindStore(InActor);
	const auto* entry = store ? store->AttackFacingByActor.Find(TWeakObjectPtr<AActor>(const_cast<AActor*>(InActor))) : nullptr;
	if (entry) { OutRecord = *entry; return true; }
#endif
	return false;
}
