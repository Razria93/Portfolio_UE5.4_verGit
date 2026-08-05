#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"

#include "Type/CCombatResultTypes.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

#if !UE_BUILD_SHIPPING
using namespace DebugOverlaySnapshotStoreInternals;

namespace
{
	uint64 GetCurrentFrameNumber()
	{
		return GFrameCounter;
	}

	float GetWorldTimeSeconds(const UWorld* InWorld)
	{
		return IsValid(InWorld) ? InWorld->GetTimeSeconds() : 0.f;
	}
}

FDebugOverlaySnapshotStamp SnapshotRecordBuilders::MakeSnapshotStamp(const UWorld* InWorld)
{
	FDebugOverlaySnapshotStamp stamp;
	stamp.FrameNumber = GetCurrentFrameNumber();
	stamp.WorldTimeSeconds = GetWorldTimeSeconds(InWorld);
	return stamp;
}

FString SnapshotRecordBuilders::ToSafeEventName(const TCHAR* InEventName, const TCHAR* InFallback)
{
	return InEventName ? FString(InEventName) : FString(InFallback);
}

FString SnapshotRecordBuilders::ToSafeReason(const TCHAR* InReason)
{
	return InReason ? FString(InReason) : FString(TEXT("None"));
}

FString SnapshotRecordBuilders::GetDisplayNameOrNA(const UObject* InObject)
{
	return IsValid(InObject) ? GetNameSafe(InObject) : FString(TEXT("N/A"));
}

FString SnapshotRecordBuilders::CompactStoreEnumText(const FString& InValue)
{
	int32 separatorIndex = INDEX_NONE;
	return InValue.FindLastChar(TEXT(':'), separatorIndex)
		&& separatorIndex > 0
		&& InValue[separatorIndex - 1] == TEXT(':')
		&& separatorIndex + 1 < InValue.Len()
		? InValue.RightChop(separatorIndex + 1)
		: InValue;
}

FString SnapshotRecordBuilders::CompactStoreReasonText(const FString& InValue)
{
	return CompactStoreEnumText(InValue.IsEmpty() ? FString(TEXT("None")) : InValue);
}

FString SnapshotRecordBuilders::ResolveCombatResultSourceName(const AActor* InResultReceiverActor, const FCombatResultPacket& InPacket)
{
	if (IsValid(InPacket.SourceActor) && InPacket.SourceActor != InResultReceiverActor)
	{
		return GetNameSafe(InPacket.SourceActor);
	}

	if (IsValid(InPacket.TargetActor) && InPacket.TargetActor != InResultReceiverActor)
	{
		return GetNameSafe(InPacket.TargetActor);
	}

	return GetDisplayNameOrNA(InPacket.SourceActor);
}

bool SnapshotRecordBuilders::IsSameCombatPair(const FDebugOverlayCombatSummary& InSummary, const FString& InSourceName, const FString& InTargetName)
{
	return InSummary.SourceName == InSourceName && InSummary.TargetName == InTargetName;
}

FDebugOverlayEventEntry SnapshotRecordBuilders::MakeEventEntry(const UWorld* InWorld, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary)
{
	const FDebugOverlaySnapshotStamp stamp = MakeSnapshotStamp(InWorld);

	FDebugOverlayEventEntry entry;
	entry.FrameNumber = stamp.FrameNumber;
	entry.WorldTimeSeconds = stamp.WorldTimeSeconds;
	entry.Category = InCategory;
	entry.EventName = InEventName;
	entry.OwnerName = InOwnerName;
	entry.SourceName = InSourceName;
	entry.TargetName = InTargetName;
	entry.Summary = InSummary;

	return entry;
}

void SnapshotRecordBuilders::RecordRecentCombatPairInternal(FDebugOverlayWorldStore& InStore, const UWorld* InWorld, AActor* InSourceActor, AActor* InTargetActor, const FString& InEventName)
{
	const FDebugOverlaySnapshotStamp stamp = MakeSnapshotStamp(InWorld);

	InStore.RecentCombatPair.SourceActor = InSourceActor;
	InStore.RecentCombatPair.TargetActor = InTargetActor;
	InStore.RecentCombatPair.SourceName = GetNameSafe(InSourceActor);
	InStore.RecentCombatPair.TargetName = GetNameSafe(InTargetActor);
	InStore.RecentCombatPair.FrameNumber = stamp.FrameNumber;
	InStore.RecentCombatPair.WorldTimeSeconds = stamp.WorldTimeSeconds;
	InStore.RecentCombatPair.EventName = InEventName;
	InStore.bHasRecentCombatPair = true;
}
#endif
