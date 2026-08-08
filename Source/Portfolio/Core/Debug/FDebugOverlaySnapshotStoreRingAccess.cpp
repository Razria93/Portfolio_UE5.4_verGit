#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"

#if !UE_BUILD_SHIPPING
using namespace DebugOverlaySnapshotStoreInternals;

// ===== Event Ring Write =====

void EventRingAccess::AddEventInternal(FDebugOverlayWorldStore& InStore, const FDebugOverlayEventEntry& InEntry)
{
	if (InStore.EventRing.Num() < EventStoreCapacity)
	{
		InStore.EventRing.Add(InEntry);
	}
	else
	{
		InStore.EventRing[InStore.NextEventIndex] = InEntry;
	}

	InStore.NextEventIndex = (InStore.NextEventIndex + 1) % EventStoreCapacity;
	InStore.EventCount = FMath::Min(InStore.EventCount + 1, EventStoreCapacity);

	InStore.Snapshot.RecentEvents = GetRecentEventsCopyFromStore(
		InStore,
		EventFilterPolicy::GetClampedEventLogDisplayLimit(),
		MaxEventLogDisplayLimit,
		TEXT("All"),
		false);
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
