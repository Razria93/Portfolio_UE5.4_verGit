#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"

#include "Engine/World.h"

#if !UE_BUILD_SHIPPING
using namespace DebugOverlaySnapshotStoreInternals;

// ===== Store State =====

namespace DebugOverlaySnapshotStoreInternals
{
	TMap<TObjectKey<UWorld>, FDebugOverlayWorldStore> GStoresByWorld;
}

// ===== World Cleanup Delegate =====

namespace
{
	void HandleWorldCleanup(UWorld* InWorld, bool, bool)
	{
		StoreLifecycle::RemoveStoreForWorld(InWorld);
	}

	void EnsureWorldCleanupDelegateRegistered()
	{
		static bool bRegistered = false;
		if (bRegistered) return;

		FWorldDelegates::OnWorldCleanup.AddStatic(&HandleWorldCleanup);
		bRegistered = true;
	}
}

// ===== World Resolution =====

UWorld* StoreLifecycle::ResolveWorld(const UObject* InWorldContextObject)
{
	if (!IsValid(InWorldContextObject)) return nullptr;

	if (UWorld* world = Cast<UWorld>(const_cast<UObject*>(InWorldContextObject)))
	{
		return world;
	}

	return InWorldContextObject->GetWorld();
}

// ===== Store Lookup =====

FDebugOverlayWorldStore* StoreLifecycle::FindStore(const UObject* InWorldContextObject)
{
	UWorld* world = ResolveWorld(InWorldContextObject);
	if (!IsValid(world)) return nullptr;

	return GStoresByWorld.Find(TObjectKey<UWorld>(world));
}

FDebugOverlayWorldStore* StoreLifecycle::FindOrAddStore(const UObject* InWorldContextObject)
{
	UWorld* world = ResolveWorld(InWorldContextObject);
	if (!IsValid(world)) return nullptr;

	EnsureWorldCleanupDelegateRegistered();
	return &GStoresByWorld.FindOrAdd(TObjectKey<UWorld>(world));
}

void StoreLifecycle::RemoveStoreForWorld(UWorld* InWorld)
{
	if (!InWorld) return;

	GStoresByWorld.Remove(TObjectKey<UWorld>(InWorld));
}

// ===== Store Reset =====

void StoreLifecycle::ResetAllStores()
{
	GStoresByWorld.Reset();
}
#endif
