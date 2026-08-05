#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"

#include "UObject/ObjectKey.h"

class UWorld;
struct FCombatResultPacket;

#if !UE_BUILD_SHIPPING
namespace DebugOverlaySnapshotStoreInternals
{
	inline constexpr int32 EventStoreCapacity = 32;
	inline constexpr int32 DefaultEventLogDisplayLimit = 16;
	inline constexpr int32 MaxEventLogDisplayLimit = 32;

	struct FDebugOverlayWorldStore
	{
		FDebugOverlaySnapshot Snapshot;
		TArray<FDebugOverlayEventEntry> EventRing;
		FDebugOverlayRecentCombatPair RecentCombatPair;
		int32 NextEventIndex = 0;
		int32 EventCount = 0;
		bool bHasRecentCombatPair = false;
	};

	struct FDebugOverlaySnapshotStamp
	{
		uint64 FrameNumber = 0;
		float WorldTimeSeconds = 0.f;
	};

	extern TMap<TObjectKey<UWorld>, FDebugOverlayWorldStore> GStoresByWorld;
}

namespace SnapshotStoreConfig
{
	bool IsEnabled();
	bool IsCollecting();
	bool ShouldHideNoiseEvents();
	bool ShouldHideCollisionWindowEvents();
	int32 GetEventLogDisplayLimitRaw();
	FString GetEventLogFilterRaw();
}

namespace StoreLifecycle
{
	UWorld* ResolveWorld(const UObject* InWorldContextObject);
	void RemoveStoreForWorld(UWorld* InWorld);
	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* FindStore(const UObject* InWorldContextObject);
	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* FindOrAddStore(const UObject* InWorldContextObject);
	void ResetAllStores();
}

namespace EventFilterPolicy
{
	FString NormalizeEventLogFilter(const FString& InFilter);
	int32 GetClampedEventLogDisplayLimit();
	FString GetCanonicalEventLogFilter();
	bool ShouldIncludeEventForDisplay(const FDebugOverlayEventEntry& InEntry, const FString& InFilter, bool bApplyDisplayFilters);
	bool DoesEventMatchSubject(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName);
	FDebugOverlayEventEntry MakeSubjectDisplayEventEntry(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName);
}

namespace EventRingAccess
{
	void AddEventInternal(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, const FDebugOverlayEventEntry& InEntry);
	TArray<FDebugOverlayEventEntry> GetRecentEventsCopyFromStore(const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, bool bApplyDisplayFilters);
	TArray<FDebugOverlayEventEntry> GetRecentEventsForSubjectCopyFromStore(const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, const FString& InSubjectName, bool bApplyDisplayFilters);
}

namespace SnapshotRecordBuilders
{
	DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp MakeSnapshotStamp(const UWorld* InWorld);
	FString ToSafeEventName(const TCHAR* InEventName, const TCHAR* InFallback);
	FString ToSafeReason(const TCHAR* InReason);
	FString GetDisplayNameOrNA(const UObject* InObject);
	FString CompactStoreEnumText(const FString& InValue);
	FString CompactStoreReasonText(const FString& InValue);
	FString ResolveCombatResultSourceName(const AActor* InResultReceiverActor, const FCombatResultPacket& InPacket);
	bool IsSameCombatPair(const FDebugOverlayCombatSummary& InSummary, const FString& InSourceName, const FString& InTargetName);
	FDebugOverlayEventEntry MakeEventEntry(const UWorld* InWorld, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary);
	void RecordRecentCombatPairInternal(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, const UWorld* InWorld, AActor* InSourceActor, AActor* InTargetActor, const FString& InEventName);
}
#endif
