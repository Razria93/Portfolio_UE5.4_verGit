#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"

#include "UObject/ObjectKey.h"

class UWorld;
class AActor;

#if !UE_BUILD_SHIPPING
// ===== Internal Store State =====

namespace DebugOverlaySnapshotStoreInternals
{
	inline constexpr int32 EventStoreCapacity = 256;
	inline constexpr int32 ActorEventHistoryCapacity = 128;
	inline constexpr int32 MaxActorEventHistories = 64;
	inline constexpr int32 MaxAIActorSummaries = 64;
	inline constexpr int32 DefaultEventLogDisplayLimit = 16;
	inline constexpr int32 MaxEventLogDisplayLimit = 32;

	struct FDebugOverlayActorEventHistory
	{
		TWeakObjectPtr<AActor> Actor;
		TArray<FDebugOverlayEventEntry> Events;
		uint64 LastWriteSerial = 0;
	};

	struct FDebugOverlayWorldStore
	{
		FDebugOverlaySnapshot Snapshot;
		TArray<FDebugOverlayEventEntry> EventRing;
		FDebugOverlayRecentCombatPair RecentCombatPair;
		TMap<TObjectKey<AActor>, FDebugOverlayActorEventHistory> EventHistoryByActor;
		TMap<TWeakObjectPtr<APawn>, FDebugOverlayAISummary> LastAIByPawn;
		int32 NextEventIndex = 0;
		int32 EventCount = 0;
		uint64 EventWriteSerial = 0;
		bool bHasRecentCombatPair = false;
	};

	struct FDebugOverlaySnapshotStamp
	{
		uint64 FrameNumber = 0;
		float WorldTimeSeconds = 0.f;
	};

	extern TMap<TObjectKey<UWorld>, FDebugOverlayWorldStore> GStoresByWorld;
}

// ===== Runtime Config Accessors =====

namespace SnapshotStoreConfig
{
	bool IsHudVisible();
	bool IsCollecting();
	int32 GetEventLogDisplayLimitRaw();
	FString GetEventLogFilterRaw();
	FString GetEventLogScopeRaw();
	bool ShouldHideNoiseEvents();
	bool ShouldHideCollisionWindowEvents();
}

// ===== Store Lifecycle =====

namespace StoreLifecycle
{
	UWorld* ResolveWorld(const UObject* InWorldContextObject);
	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* FindStore(const UObject* InWorldContextObject);
	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* FindOrAddStore(const UObject* InWorldContextObject);
	void RemoveStoreForWorld(UWorld* InWorld);
	void ResetAllStores();
}

// ===== Snapshot Record Builders =====

namespace SnapshotRecordBuilders
{
	FString FormatEventNameOrFallback(const TCHAR* InEventName, const TCHAR* InFallback);
	FString FormatReasonOrNone(const TCHAR* InReason);
	FString FormatDisplayNameOrNA(const UObject* InObject);
	FString FormatCompactEnumText(const FString& InValue);
	FString FormatCompactReasonText(const FString& InValue);
	DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp MakeSnapshotStamp(const UWorld* InWorld);
	FDebugOverlayEventEntry MakeEventEntry(const UWorld* InWorld, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary);
	FDebugOverlayEventEntry MakeCombatEventEntry(const UWorld* InWorld, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FDebugOverlayCombatEventDetails& InCombatDetails);
	void UpdateRecentCombatPair(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, const UWorld* InWorld, AActor* InSourceActor, AActor* InTargetActor, const FString& InEventName);
}

// ===== Event Filter Policy =====

namespace EventFilterPolicy
{
	FString NormalizeEventLogFilter(const FString& InFilter);
	FString GetCanonicalEventLogFilter();
	FString NormalizeEventLogScope(const FString& InScope);
	FString GetCanonicalEventLogScope();
	int32 GetClampedEventLogDisplayLimit();
	bool ShouldIncludeEventForDisplay(const FDebugOverlayEventEntry& InEntry, const FString& InFilter, bool bApplyDisplayFilters);
}

// ===== Event Ring Access =====

namespace EventRingAccess
{
	void AddEventInternal(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, const FDebugOverlayEventEntry& InEntry, const AActor* InOwnerActor = nullptr, const AActor* InSourceActor = nullptr, const AActor* InTargetActor = nullptr);
	TArray<FDebugOverlayEventEntry> GetRecentEventsCopyFromStore(const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, bool bApplyDisplayFilters);
	TArray<FDebugOverlayEventEntry> GetRecentEventsForActorCopyFromStore(const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, const AActor* InActor, bool bApplyDisplayFilters);
	void RemoveActorEventHistoryFromStore(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, const AActor* InActor);
}
#endif
