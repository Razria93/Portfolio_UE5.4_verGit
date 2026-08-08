#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"

#include "Type/CCombatResultTypes.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"

#if !UE_BUILD_SHIPPING
namespace
{
	// ===== Constants =====

	constexpr TCHAR NoneText[] = TEXT("None");
	constexpr TCHAR NotAvailableText[] = TEXT("N/A");

	// ===== Time Helpers =====

	uint64 GetCurrentFrameNumber()
	{
		return GFrameCounter;
	}

	float GetWorldTimeSeconds(const UWorld* InWorld)
	{
		return IsValid(InWorld) ? InWorld->GetTimeSeconds() : 0.f;
	}
}

// ===== Text Helpers =====

FString SnapshotRecordBuilders::FormatEventNameOrFallback(const TCHAR* InEventName, const TCHAR* InFallback)
{
	return InEventName ? FString(InEventName) : FString(InFallback);
}

FString SnapshotRecordBuilders::FormatDisplayNameOrNA(const UObject* InObject)
{
	return IsValid(InObject) ? GetNameSafe(InObject) : FString(NotAvailableText);
}

FString SnapshotRecordBuilders::FormatReasonOrNone(const TCHAR* InReason)
{
	return InReason ? FString(InReason) : FString(NoneText);
}

FString SnapshotRecordBuilders::FormatCompactEnumText(const FString& InValue)
{
	int32 separatorIndex = INDEX_NONE;
	return InValue.FindLastChar(TEXT(':'), separatorIndex)
		&& separatorIndex > 0
		&& InValue[separatorIndex - 1] == TEXT(':')
		&& separatorIndex + 1 < InValue.Len()
		? InValue.RightChop(separatorIndex + 1)
		: InValue;
}

FString SnapshotRecordBuilders::FormatCompactReasonText(const FString& InValue)
{
	return FormatCompactEnumText(InValue.IsEmpty() ? FString(NoneText) : InValue);
}

// ===== Combat Helpers =====

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

	return FormatDisplayNameOrNA(InPacket.SourceActor);
}

bool SnapshotRecordBuilders::IsSameCombatPair(const FDebugOverlayCombatSummary& InSummary, const FString& InSourceName, const FString& InTargetName)
{
	return InSummary.SourceName == InSourceName && InSummary.TargetName == InTargetName;
}

// ===== Snapshot Stamp =====

DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp SnapshotRecordBuilders::MakeSnapshotStamp(const UWorld* InWorld)
{
	DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp;
	stamp.FrameNumber = GetCurrentFrameNumber();
	stamp.WorldTimeSeconds = GetWorldTimeSeconds(InWorld);
	return stamp;
}

// ===== Event Entry Builders =====

FDebugOverlayEventEntry SnapshotRecordBuilders::MakeEventEntry(const UWorld* InWorld, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary)
{
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = MakeSnapshotStamp(InWorld);

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

// ===== Recent Combat Pair =====

void SnapshotRecordBuilders::UpdateRecentCombatPair(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, const UWorld* InWorld, AActor* InSourceActor, AActor* InTargetActor, const FString& InEventName)
{
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = MakeSnapshotStamp(InWorld);

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
