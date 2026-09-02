#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"

#include "GameFramework/Actor.h"

#if !UE_BUILD_SHIPPING
using namespace DebugOverlaySnapshotStoreInternals;

// ===== Event Ring Write =====

namespace
{
	void PruneInvalidActorHistories(FDebugOverlayWorldStore& InStore)
	{
		for (auto it = InStore.EventHistoryByActor.CreateIterator(); it; ++it)
		{
			if (!it.Value().Actor.IsValid())
			{
				it.RemoveCurrent();
			}
		}
	}

	void EnforceActorHistoryCapacity(FDebugOverlayWorldStore& InStore)
	{
		while (InStore.EventHistoryByActor.Num() >= MaxActorEventHistories)
		{
			TOptional<TObjectKey<AActor>> oldestKey;
			uint64 oldestSerial = MAX_uint64;
			for (const TPair<TObjectKey<AActor>, FDebugOverlayActorEventHistory>& pair : InStore.EventHistoryByActor)
			{
				if (pair.Value.LastWriteSerial < oldestSerial)
				{
					oldestKey = pair.Key;
					oldestSerial = pair.Value.LastWriteSerial;
				}
			}

			if (!oldestKey.IsSet()) return;
			InStore.EventHistoryByActor.Remove(oldestKey.GetValue());
		}
	}

	void AddActorHistoryEvent(FDebugOverlayWorldStore& InStore, const AActor* InActor, const FDebugOverlayEventEntry& InEntry)
	{
		if (!IsValid(InActor)) return;

		const TObjectKey<AActor> actorKey(const_cast<AActor*>(InActor));
		FDebugOverlayActorEventHistory* history = InStore.EventHistoryByActor.Find(actorKey);
		if (!history)
		{
			EnforceActorHistoryCapacity(InStore);
			history = &InStore.EventHistoryByActor.FindOrAdd(actorKey);
			history->Actor = const_cast<AActor*>(InActor);
		}

		history->Events.Add(InEntry);
		if (history->Events.Num() > ActorEventHistoryCapacity)
		{
			history->Events.RemoveAt(0, history->Events.Num() - ActorEventHistoryCapacity, EAllowShrinking::No);
		}
		history->LastWriteSerial = ++InStore.EventWriteSerial;
	}
}

void EventRingAccess::AddEventInternal(FDebugOverlayWorldStore& InStore, const FDebugOverlayEventEntry& InEntry, const AActor* InOwnerActor, const AActor* InSourceActor, const AActor* InTargetActor)
{
	PruneInvalidActorHistories(InStore);
	FDebugOverlayEventEntry entry = InEntry;
	entry.OwnerActor = const_cast<AActor*>(InOwnerActor);
	entry.SourceActor = const_cast<AActor*>(InSourceActor);
	entry.TargetActor = const_cast<AActor*>(InTargetActor);

	if (InStore.EventRing.Num() < EventStoreCapacity)
	{
		InStore.EventRing.Add(entry);
	}
	else
	{
		InStore.EventRing[InStore.NextEventIndex] = entry;
	}

	InStore.NextEventIndex = (InStore.NextEventIndex + 1) % EventStoreCapacity;
	InStore.EventCount = FMath::Min(InStore.EventCount + 1, EventStoreCapacity);

	AddActorHistoryEvent(InStore, InOwnerActor, entry);
	if (InSourceActor != InOwnerActor)
	{
		AddActorHistoryEvent(InStore, InSourceActor, entry);
	}
	if (InTargetActor != InOwnerActor && InTargetActor != InSourceActor)
	{
		AddActorHistoryEvent(InStore, InTargetActor, entry);
	}

	InStore.Snapshot.RecentEvents = GetRecentEventsCopyFromStore(
		InStore,
		EventFilterPolicy::GetClampedEventLogDisplayLimit(),
		MaxEventLogDisplayLimit,
		TEXT("All"),
		false);
}

TArray<FDebugOverlayEventEntry> EventRingAccess::GetRecentEventsForActorCopyFromStore(const FDebugOverlayWorldStore& InStore, const int32 InMaxEvents, const int32 InMaxClamp, const FString& InFilter, const AActor* InActor, const bool bApplyDisplayFilters)
{
	TArray<FDebugOverlayEventEntry> result;
	if (!IsValid(InActor)) return result;

	const FDebugOverlayActorEventHistory* history = InStore.EventHistoryByActor.Find(TObjectKey<AActor>(const_cast<AActor*>(InActor)));
	if (!history) return result;

	const int32 maxEvents = FMath::Clamp(InMaxEvents, 0, InMaxClamp);
	result.Reserve(maxEvents);
	for (int32 index = history->Events.Num() - 1; index >= 0 && result.Num() < maxEvents; --index)
	{
		const FDebugOverlayEventEntry& entry = history->Events[index];
		if (EventFilterPolicy::ShouldIncludeEventForDisplay(entry, InFilter, bApplyDisplayFilters))
		{
			result.Add(entry);
		}
	}

	return result;
}

void EventRingAccess::RemoveActorEventHistoryFromStore(FDebugOverlayWorldStore& InStore, const AActor* InActor)
{
	if (!InActor) return;
	InStore.EventHistoryByActor.Remove(TObjectKey<AActor>(const_cast<AActor*>(InActor)));
}

// ===== Event Ring Read =====

TArray<FDebugOverlayEventEntry> EventRingAccess::GetRecentEventsCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, bool bApplyDisplayFilters)
{
	TArray<FDebugOverlayEventEntry> result;

	const int32 maxEvents = FMath::Clamp(InMaxEvents, 0, InMaxClamp);
	result.Reserve(maxEvents);

	for (int32 i = 0; i < InStore.EventCount && result.Num() < maxEvents; ++i)
	{
		const int32 index = (InStore.NextEventIndex - 1 - i + EventStoreCapacity) % EventStoreCapacity;
		if (InStore.EventRing.IsValidIndex(index)
			&& EventFilterPolicy::ShouldIncludeEventForDisplay(InStore.EventRing[index], InFilter, bApplyDisplayFilters))
		{
			result.Add(InStore.EventRing[index]);
		}
	}

	return result;
}

TArray<FDebugOverlayEventEntry> EventRingAccess::GetRecentEventsForSubjectCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, const FString& InSubjectName, bool bApplyDisplayFilters)
{
	TArray<FDebugOverlayEventEntry> result;
	if (InSubjectName.IsEmpty()) return result;

	const int32 maxEvents = FMath::Clamp(InMaxEvents, 0, InMaxClamp);
	result.Reserve(maxEvents);

	for (int32 i = 0; i < InStore.EventCount && result.Num() < maxEvents; ++i)
	{
		const int32 index = (InStore.NextEventIndex - 1 - i + EventStoreCapacity) % EventStoreCapacity;
		if (!InStore.EventRing.IsValidIndex(index)) continue;

		const FDebugOverlayEventEntry& entry = InStore.EventRing[index];
		if (EventFilterPolicy::ShouldIncludeEventForDisplay(entry, InFilter, bApplyDisplayFilters)
			&& EventFilterPolicy::DoesEventMatchSubject(entry, InSubjectName))
		{
			result.Add(EventFilterPolicy::MakeSubjectDisplayEventEntry(entry, InSubjectName));
		}
	}

	return result;
}
#endif
