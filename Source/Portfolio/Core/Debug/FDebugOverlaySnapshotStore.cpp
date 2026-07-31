#include "Core/Debug/FDebugOverlaySnapshotStore.h"

#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalTargetTypes.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "UObject/ObjectKey.h"

namespace
{
	static constexpr int32 DebugOverlayEventStoreCapacity = 32;
	static constexpr int32 DebugOverlayDefaultEventLogDisplayLimit = 16;
	static constexpr int32 DebugOverlayMaxEventLogDisplayLimit = 32;

#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarDebugOverlayEnabled(
		TEXT("Portfolio.DebugOverlay.Enabled"),
		0,
		TEXT("Draw debug overlay evidence HUD. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayCollect(
		TEXT("Portfolio.DebugOverlay.Collect"),
		0,
		TEXT("Collect debug overlay snapshot evidence. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayPreset(
		TEXT("Portfolio.DebugOverlay.Preset"),
		0,
		TEXT("Select debug overlay display preset. 0: P0 minimum."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayEventLogLimit(
		TEXT("Portfolio.DebugOverlay.EventLogLimit"),
		DebugOverlayDefaultEventLogDisplayLimit,
		TEXT("Number of recent debug overlay event lines to display. 0-32."),
		ECVF_Default);

	TAutoConsoleVariable<FString> CVarDebugOverlayEventLogFilter(
		TEXT("Portfolio.DebugOverlay.EventLogFilter"),
		TEXT("All"),
		TEXT("Filter debug overlay event log. Values: All, Execution, Combat, AI."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayHideNoiseEvents(
		TEXT("Portfolio.DebugOverlay.HideNoiseEvents"),
		0,
		TEXT("Hide noisy debug overlay event log entries. 0: show all, 1: hide reject/ignore noise."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDebugOverlayHideCollisionWindowEvents(
		TEXT("Portfolio.DebugOverlay.HideCollisionWindowEvents"),
		0,
		TEXT("Hide debug overlay collision window event log entries. 0: show all, 1: hide collision window events."),
		ECVF_Default);

	struct FDebugOverlayWorldStore
	{
		FDebugOverlaySnapshot Snapshot;
		TArray<FDebugOverlayEventEntry> EventRing;
		FDebugOverlayRecentCombatPair RecentCombatPair;
		int32 NextEventIndex = 0;
		int32 EventCount = 0;
		bool bHasRecentCombatPair = false;
	};

	TMap<TObjectKey<UWorld>, FDebugOverlayWorldStore> StoresByWorld;

	int32 GetClampedEventLogDisplayLimit();
	FString GetCanonicalEventLogFilter();
	TArray<FDebugOverlayEventEntry> GetRecentEventsCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, bool bApplyDisplayFilters);
	TArray<FDebugOverlayEventEntry> GetRecentEventsForSubjectCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, const FString& InSubjectName, bool bApplyDisplayFilters);

	UWorld* ResolveWorld(const UObject* InWorldContextObject)
	{
		if (!IsValid(InWorldContextObject)) return nullptr;

		if (UWorld* world = Cast<UWorld>(const_cast<UObject*>(InWorldContextObject)))
		{
			return world;
		}

		return InWorldContextObject->GetWorld();
	}

	uint64 GetCurrentFrameNumber()
	{
		return GFrameCounter;
	}

	float GetWorldTimeSeconds(const UWorld* InWorld)
	{
		return IsValid(InWorld) ? InWorld->GetTimeSeconds() : 0.f;
	}

	FString ToSafeEventName(const TCHAR* InEventName, const TCHAR* InFallback)
	{
		return InEventName ? FString(InEventName) : FString(InFallback);
	}

	FString ToSafeReason(const TCHAR* InReason)
	{
		return InReason ? FString(InReason) : FString(TEXT("None"));
	}

	FString GetDisplayNameOrNA(const UObject* InObject)
	{
		return IsValid(InObject) ? GetNameSafe(InObject) : FString(TEXT("N/A"));
	}

	FString CompactStoreEnumText(const FString& InValue)
	{
		int32 separatorIndex = INDEX_NONE;
		return InValue.FindLastChar(TEXT(':'), separatorIndex)
			&& separatorIndex > 0
			&& InValue[separatorIndex - 1] == TEXT(':')
			&& separatorIndex + 1 < InValue.Len()
			? InValue.RightChop(separatorIndex + 1)
			: InValue;
	}

	FString CompactReasonText(const FString& InValue)
	{
		return CompactStoreEnumText(InValue.IsEmpty() ? FString(TEXT("None")) : InValue);
	}

	FString ResolveCombatResultSourceName(const AActor* InResultReceiverActor, const FCombatResultPacket& InPacket)
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

	bool IsSameCombatPair(const FDebugOverlayCombatSummary& InSummary, const FString& InSourceName, const FString& InTargetName)
	{
		return InSummary.SourceName == InSourceName && InSummary.TargetName == InTargetName;
	}

	FString NormalizeEventLogFilter(const FString& InFilter)
	{
		if (InFilter.Equals(TEXT("Execution"), ESearchCase::IgnoreCase))
		{
			return TEXT("Execution");
		}

		if (InFilter.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
		{
			return TEXT("Combat");
		}

		if (InFilter.Equals(TEXT("AI"), ESearchCase::IgnoreCase))
		{
			return TEXT("AI");
		}

		return TEXT("All");
	}

	bool DoesEventMatchFilter(const FDebugOverlayEventEntry& InEntry, const FString& InFilter)
	{
		const FString filter = NormalizeEventLogFilter(InFilter);
		if (filter == TEXT("All"))
		{
			return true;
		}

		if (filter == TEXT("Combat"))
		{
			return InEntry.Category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)
				|| InEntry.Category.Equals(TEXT("CombatResult"), ESearchCase::IgnoreCase);
		}

		return InEntry.Category.Equals(filter, ESearchCase::IgnoreCase);
	}

	FString ExtractSummaryFieldValue(const FString& InSummary, const FString& InFieldName)
	{
		TArray<FString> summaryParts;
		InSummary.ParseIntoArray(summaryParts, TEXT("|"), true);

		for (FString summaryPart : summaryParts)
		{
			summaryPart.TrimStartAndEndInline();

			const FString colonPrefix = FString::Printf(TEXT("%s:"), *InFieldName);
			if (summaryPart.StartsWith(colonPrefix, ESearchCase::IgnoreCase))
			{
				FString value = summaryPart.RightChop(colonPrefix.Len());
				value.TrimStartAndEndInline();
				return value;
			}

			const FString equalsPrefix = FString::Printf(TEXT("%s="), *InFieldName);
			if (summaryPart.StartsWith(equalsPrefix, ESearchCase::IgnoreCase))
			{
				FString value = summaryPart.RightChop(equalsPrefix.Len());
				value.TrimStartAndEndInline();
				return value;
			}
		}

		return FString();
	}

	bool IsExecutionNoiseEvent(const FDebugOverlayEventEntry& InEntry)
	{
		if (!InEntry.Category.Equals(TEXT("Execution"), ESearchCase::IgnoreCase)) return false;
		if (!InEntry.EventName.Equals(TEXT("DecisionResolved"), ESearchCase::IgnoreCase)) return false;

		const FString decision = ExtractSummaryFieldValue(InEntry.Summary, TEXT("Decision"));
		if (decision.Equals(TEXT("Reject"), ESearchCase::IgnoreCase)
			|| decision.Equals(TEXT("Ignore"), ESearchCase::IgnoreCase))
		{
			return true;
		}

		const FString rejectReason = ExtractSummaryFieldValue(InEntry.Summary, TEXT("RejectReason"));
		return !rejectReason.IsEmpty() && !rejectReason.Equals(TEXT("None"), ESearchCase::IgnoreCase);
	}

	bool IsCollisionWindowEvent(const FDebugOverlayEventEntry& InEntry)
	{
		const FString category = InEntry.Category.TrimStartAndEnd();
		if (!category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)) return false;

		const FString eventName = InEntry.EventName.TrimStartAndEnd();
		return eventName.StartsWith(TEXT("CollisionEnabled"), ESearchCase::IgnoreCase)
			|| eventName.StartsWith(TEXT("CollisionDisabled"), ESearchCase::IgnoreCase)
			|| eventName.StartsWith(TEXT("CollisionDisableIgnored"), ESearchCase::IgnoreCase);
	}

	bool IsCollisionDisableIgnoredEvent(const FDebugOverlayEventEntry& InEntry)
	{
		const FString category = InEntry.Category.TrimStartAndEnd();
		if (!category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)) return false;

		const FString eventName = InEntry.EventName.TrimStartAndEnd();
		return eventName.StartsWith(TEXT("CollisionDisableIgnored"), ESearchCase::IgnoreCase)
			|| eventName.StartsWith(TEXT("CollisionDisabledIgnored"), ESearchCase::IgnoreCase);
	}

	bool IsEventExcludedByDisplayFilters(const FDebugOverlayEventEntry& InEntry)
	{
		const bool bHideNoiseEvents = CVarDebugOverlayHideNoiseEvents.GetValueOnGameThread() != 0;
		const bool bHideCollisionWindowEvents = CVarDebugOverlayHideCollisionWindowEvents.GetValueOnGameThread() != 0;

		if (bHideNoiseEvents)
		{
			if (IsExecutionNoiseEvent(InEntry)) return true;
			if (IsCollisionDisableIgnoredEvent(InEntry)) return true;
		}

		if (bHideCollisionWindowEvents && IsCollisionWindowEvent(InEntry))
		{
			return true;
		}

		return false;
	}

	bool ShouldIncludeEventForDisplay(const FDebugOverlayEventEntry& InEntry, const FString& InFilter, bool bApplyDisplayFilters)
	{
		if (!DoesEventMatchFilter(InEntry, InFilter)) return false;
		if (bApplyDisplayFilters && IsEventExcludedByDisplayFilters(InEntry)) return false;

		return true;
	}

	bool DoesEventMatchSubject(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName)
	{
		if (InSubjectName.IsEmpty()) return false;

		const bool bMatchesAnyRole =
			InEntry.OwnerName == InSubjectName
			|| InEntry.SourceName == InSubjectName
			|| InEntry.TargetName == InSubjectName;

		if (InEntry.Category.Equals(TEXT("Execution"), ESearchCase::IgnoreCase))
		{
			return InEntry.OwnerName == InSubjectName;
		}

		if (InEntry.Category.Equals(TEXT("AI"), ESearchCase::IgnoreCase))
		{
			return InEntry.OwnerName == InSubjectName
				|| InEntry.SourceName == InSubjectName;
		}

		if (InEntry.Category.Equals(TEXT("CombatResult"), ESearchCase::IgnoreCase))
		{
			return InEntry.OwnerName == InSubjectName
				|| InEntry.TargetName == InSubjectName;
		}

		if (InEntry.Category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
		{
			if (InEntry.EventName.Contains(TEXT("Collision"), ESearchCase::IgnoreCase))
			{
				return InEntry.OwnerName == InSubjectName
					|| InEntry.SourceName == InSubjectName;
			}

			if (InEntry.EventName.Contains(TEXT("TargetAccepted"), ESearchCase::IgnoreCase)
				|| InEntry.EventName.Contains(TEXT("TargetRejected"), ESearchCase::IgnoreCase))
			{
				return bMatchesAnyRole;
			}

			return bMatchesAnyRole;
		}

		return bMatchesAnyRole;
	}

	bool IsTargetPacketEvent(const FDebugOverlayEventEntry& InEntry)
	{
		return InEntry.Category.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)
			&& (InEntry.EventName.Contains(TEXT("TargetAccepted"), ESearchCase::IgnoreCase)
				|| InEntry.EventName.Contains(TEXT("TargetRejected"), ESearchCase::IgnoreCase));
	}

	FString GetSubjectEventRoleLabel(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName)
	{
		if (!IsTargetPacketEvent(InEntry) || InSubjectName.IsEmpty()) return FString();

		const bool bIsSource = InEntry.SourceName == InSubjectName;
		const bool bIsTarget = InEntry.TargetName == InSubjectName;
		const bool bIsOwner = InEntry.OwnerName == InSubjectName;

		if (bIsSource && bIsTarget)
		{
			return TEXT("Self");
		}

		if (bIsSource)
		{
			return TEXT("Outgoing");
		}

		if (bIsTarget || bIsOwner)
		{
			return TEXT("Incoming");
		}

		return FString();
	}

	FDebugOverlayEventEntry MakeSubjectDisplayEventEntry(const FDebugOverlayEventEntry& InEntry, const FString& InSubjectName)
	{
		FDebugOverlayEventEntry entry = InEntry;
		const FString roleLabel = GetSubjectEventRoleLabel(entry, InSubjectName);
		if (!roleLabel.IsEmpty())
		{
			entry.EventName = FString::Printf(TEXT("%s(%s)"), *entry.EventName, *roleLabel);
		}

		return entry;
	}

	FDebugOverlayEventEntry MakeEventEntry(const UWorld* InWorld, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary)
	{
		FDebugOverlayEventEntry entry;
		entry.FrameNumber = GetCurrentFrameNumber();
		entry.WorldTimeSeconds = GetWorldTimeSeconds(InWorld);
		entry.Category = InCategory;
		entry.EventName = InEventName;
		entry.OwnerName = InOwnerName;
		entry.SourceName = InSourceName;
		entry.TargetName = InTargetName;
		entry.Summary = InSummary;

		return entry;
	}

	void AddEventInternal(FDebugOverlayWorldStore& InStore, const FDebugOverlayEventEntry& InEntry)
	{
		if (InStore.EventRing.Num() < DebugOverlayEventStoreCapacity)
		{
			InStore.EventRing.Add(InEntry);
		}
		else
		{
			InStore.EventRing[InStore.NextEventIndex] = InEntry;
		}

		InStore.NextEventIndex = (InStore.NextEventIndex + 1) % DebugOverlayEventStoreCapacity;
		InStore.EventCount = FMath::Min(InStore.EventCount + 1, DebugOverlayEventStoreCapacity);

		InStore.Snapshot.RecentEvents = GetRecentEventsCopyFromStore(InStore, GetClampedEventLogDisplayLimit(), DebugOverlayMaxEventLogDisplayLimit, TEXT("All"), false);
	}

	void RecordRecentCombatPairInternal(FDebugOverlayWorldStore& InStore, const UWorld* InWorld, AActor* InSourceActor, AActor* InTargetActor, const FString& InEventName)
	{
		InStore.RecentCombatPair.SourceActor = InSourceActor;
		InStore.RecentCombatPair.TargetActor = InTargetActor;
		InStore.RecentCombatPair.SourceName = GetNameSafe(InSourceActor);
		InStore.RecentCombatPair.TargetName = GetNameSafe(InTargetActor);
		InStore.RecentCombatPair.FrameNumber = GetCurrentFrameNumber();
		InStore.RecentCombatPair.WorldTimeSeconds = GetWorldTimeSeconds(InWorld);
		InStore.RecentCombatPair.EventName = InEventName;
		InStore.bHasRecentCombatPair = true;
	}

	TArray<FDebugOverlayEventEntry> GetRecentEventsCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, bool bApplyDisplayFilters)
	{
		TArray<FDebugOverlayEventEntry> result;

		const int32 maxEvents = FMath::Clamp(InMaxEvents, 0, InMaxClamp);
		result.Reserve(maxEvents);

		for (int32 i = 0; i < InStore.EventCount && result.Num() < maxEvents; ++i)
		{
			const int32 index = (InStore.NextEventIndex - 1 - i + DebugOverlayEventStoreCapacity) % DebugOverlayEventStoreCapacity;
			if (InStore.EventRing.IsValidIndex(index) && ShouldIncludeEventForDisplay(InStore.EventRing[index], InFilter, bApplyDisplayFilters))
			{
				result.Add(InStore.EventRing[index]);
			}
		}

		return result;
	}

	TArray<FDebugOverlayEventEntry> GetRecentEventsForSubjectCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp, const FString& InFilter, const FString& InSubjectName, bool bApplyDisplayFilters)
	{
		TArray<FDebugOverlayEventEntry> result;
		if (InSubjectName.IsEmpty()) return result;

		const int32 maxEvents = FMath::Clamp(InMaxEvents, 0, InMaxClamp);
		result.Reserve(maxEvents);

		for (int32 i = 0; i < InStore.EventCount && result.Num() < maxEvents; ++i)
		{
			const int32 index = (InStore.NextEventIndex - 1 - i + DebugOverlayEventStoreCapacity) % DebugOverlayEventStoreCapacity;
			if (!InStore.EventRing.IsValidIndex(index)) continue;

			const FDebugOverlayEventEntry& entry = InStore.EventRing[index];
			if (ShouldIncludeEventForDisplay(entry, InFilter, bApplyDisplayFilters) && DoesEventMatchSubject(entry, InSubjectName))
			{
				result.Add(MakeSubjectDisplayEventEntry(entry, InSubjectName));
			}
		}

		return result;
	}

	int32 GetClampedEventLogDisplayLimit()
	{
		return FMath::Clamp(
			CVarDebugOverlayEventLogLimit.GetValueOnGameThread(),
			0,
			DebugOverlayMaxEventLogDisplayLimit);
	}

	FString GetCanonicalEventLogFilter()
	{
		return NormalizeEventLogFilter(CVarDebugOverlayEventLogFilter.GetValueOnGameThread());
	}

	FDebugOverlayWorldStore* FindStore(const UObject* InWorldContextObject)
	{
		UWorld* world = ResolveWorld(InWorldContextObject);
		if (!IsValid(world)) return nullptr;

		return StoresByWorld.Find(TObjectKey<UWorld>(world));
	}

	FDebugOverlayWorldStore* FindOrAddStore(const UObject* InWorldContextObject)
	{
		UWorld* world = ResolveWorld(InWorldContextObject);
		if (!IsValid(world)) return nullptr;

		return &StoresByWorld.FindOrAdd(TObjectKey<UWorld>(world));
	}
#endif
}

// Gate

bool FDebugOverlaySnapshotStore::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarDebugOverlayEnabled.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FDebugOverlaySnapshotStore::IsCollecting()
{
#if !UE_BUILD_SHIPPING
	return CVarDebugOverlayCollect.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

int32 FDebugOverlaySnapshotStore::GetEventLogDisplayLimit()
{
#if !UE_BUILD_SHIPPING
	return GetClampedEventLogDisplayLimit();
#else
	return 0;
#endif
}

FString FDebugOverlaySnapshotStore::GetEventLogFilter()
{
#if !UE_BUILD_SHIPPING
	return GetCanonicalEventLogFilter();
#else
	return TEXT("All");
#endif
}

// Execution Record

void FDebugOverlaySnapshotStore::RecordExecutionDecision(const UObject* InWorldContextObject, const AActor* InOwnerActor, const FString& InDomain, const FString& InSubject, const FString& InDecision, const FString& InApplyMode, const FString& InRejectReason, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	FDebugOverlayWorldStore* store = FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = ResolveWorld(InWorldContextObject);
	const FString eventName = ToSafeEventName(InEventName, TEXT("ExecutionDecision"));
	const FString ownerName = GetNameSafe(InOwnerActor);
	const FString summary = FString::Printf(
		TEXT("Owner: %s | Domain: %s | Subject: %s | Decision: %s | Apply: %s | RejectReason: %s"),
		*GetDisplayNameOrNA(InOwnerActor),
		*CompactStoreEnumText(InDomain),
		InSubject.IsEmpty() ? TEXT("N/A") : *InSubject,
		*CompactStoreEnumText(InDecision),
		*CompactStoreEnumText(InApplyMode),
		*CompactReasonText(InRejectReason));

	store->Snapshot.LastExecution.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastExecution.FrameNumber = GetCurrentFrameNumber();
	store->Snapshot.LastExecution.WorldTimeSeconds = GetWorldTimeSeconds(world);
	store->Snapshot.LastExecution.OwnerName = ownerName;
	store->Snapshot.LastExecution.Domain = InDomain;
	store->Snapshot.LastExecution.Decision = InDecision;
	store->Snapshot.LastExecution.ApplyMode = InApplyMode;
	store->Snapshot.LastExecution.RejectReason = InRejectReason;
	store->Snapshot.LastExecution.Summary = summary;

	AddEventInternal(*store, MakeEventEntry(world, TEXT("Execution"), eventName, ownerName, FString(), FString(), summary));
#endif
}

// Combat Record

void FDebugOverlaySnapshotStore::RecordWeaponCollisionWindow(const UObject* InWorldContextObject, const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, const FString& InHitWindowState, const TCHAR* InEventName, const TCHAR* InReason)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	FDebugOverlayWorldStore* store = FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = ResolveWorld(InWorldContextObject);
	const FString eventName = ToSafeEventName(InEventName, TEXT("WeaponCollisionWindow"));
	const FString ownerName = GetNameSafe(InOwnerActor);
	const FString summary = FString::Printf(
		TEXT("State: %s | HitWindow: %d | Collision: %s | Reason: %s"),
		*CompactStoreEnumText(InHitWindowState),
		InHitWindowId,
		*InCollisionName.ToString(),
		*CompactReasonText(ToSafeReason(InReason)));

	AddEventInternal(*store, MakeEventEntry(world, TEXT("Combat"), eventName, ownerName, ownerName, FString(), summary));
#endif
}

void FDebugOverlaySnapshotStore::RecordCombatTargetPacket(const UObject* InWorldContextObject, const FCombatSignalTargetPacket& InPacket, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	FDebugOverlayWorldStore* store = FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = ResolveWorld(InWorldContextObject);
	const FString eventName = ToSafeEventName(InEventName, TEXT("CombatTargetPacket"));
	const FString sourceName = GetNameSafe(InPacket.Context.SourceActor);
	const FString targetName = GetNameSafe(InPacket.Context.TargetActor);
	const FString causerName = GetNameSafe(InPacket.Context.DamageCauser);
	const FString outcome = UEnum::GetValueAsString(InPacket.Result.DefenseOutcome);
	const FString summary = FString::Printf(
		TEXT("Attacker: %s | Defender: %s | Outcome: %s | Request: %.3f | Mitigated: %.3f | Final: %.3f | Commit: %.3f | Accepted: %s"),
		*GetDisplayNameOrNA(InPacket.Context.SourceActor),
		*GetDisplayNameOrNA(InPacket.Context.TargetActor),
		*CompactStoreEnumText(outcome),
		InPacket.Result.RequestDamage,
		InPacket.Result.MitigatedDamage,
		InPacket.Result.FinalTakenDamage,
		InPacket.Result.CommittedDamage,
		InPacket.Result.bAccepted ? TEXT("true") : TEXT("false"));

	store->Snapshot.LastCombat.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastCombat.FrameNumber = GetCurrentFrameNumber();
	store->Snapshot.LastCombat.WorldTimeSeconds = GetWorldTimeSeconds(world);
	store->Snapshot.LastCombat.SourceName = sourceName;
	store->Snapshot.LastCombat.TargetName = targetName;
	store->Snapshot.LastCombat.DamageCauserName = causerName;
	store->Snapshot.LastCombat.DefenseOutcome = outcome;
	store->Snapshot.LastCombat.bHasDamageCommit = true;
	store->Snapshot.LastCombat.bDamageCommitted = !FMath::IsNearlyZero(InPacket.Result.CommittedDamage);
	store->Snapshot.LastCombat.bHasDamageBreakdown = true;
	store->Snapshot.LastCombat.RequestDamage = InPacket.Result.RequestDamage;
	store->Snapshot.LastCombat.MitigatedDamage = InPacket.Result.MitigatedDamage;
	store->Snapshot.LastCombat.FinalTakenDamage = InPacket.Result.FinalTakenDamage;
	store->Snapshot.LastCombat.CommittedDamage = InPacket.Result.CommittedDamage;
	store->Snapshot.LastCombat.Summary = summary;
	RecordRecentCombatPairInternal(*store, world, InPacket.Context.SourceActor, InPacket.Context.TargetActor, eventName);

	AddEventInternal(*store, MakeEventEntry(world, TEXT("Combat"), eventName, targetName, sourceName, targetName, summary));
#endif
}

void FDebugOverlaySnapshotStore::RecordCombatResult(const UObject* InWorldContextObject, const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	FDebugOverlayWorldStore* store = FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = ResolveWorld(InWorldContextObject);
	const FString eventName = ToSafeEventName(InEventName, TEXT("CombatResult"));
	const FString receiverName = GetNameSafe(InReceiverActor);
	const FString sourceName = GetNameSafe(InPacket.SourceActor);
	const FString targetName = GetNameSafe(InPacket.TargetActor);
	const FString causerName = GetNameSafe(InPacket.DamageCauser);
	const FString outcome = UEnum::GetValueAsString(InPacket.DefenseOutcome);
	const FString resultSourceName = ResolveCombatResultSourceName(InReceiverActor, InPacket);
	const bool bHasPreviousDamageBreakdown = store->Snapshot.LastCombat.bHasDamageBreakdown
		&& IsSameCombatPair(store->Snapshot.LastCombat, sourceName, targetName);
	const float requestDamage = bHasPreviousDamageBreakdown ? store->Snapshot.LastCombat.RequestDamage : 0.f;
	const float mitigatedDamage = bHasPreviousDamageBreakdown ? store->Snapshot.LastCombat.MitigatedDamage : 0.f;
	const float finalTakenDamage = bHasPreviousDamageBreakdown ? store->Snapshot.LastCombat.FinalTakenDamage : 0.f;
	const FString summary = bHasPreviousDamageBreakdown
		? FString::Printf(
			TEXT("ResultFrom: %s | ResultReceiver: %s | Outcome: %s | Request: %.3f | Mitigated: %.3f | Final: %.3f | Commit: %.3f | DamageCommitted: %s"),
			*resultSourceName,
			*GetDisplayNameOrNA(InReceiverActor),
			*CompactStoreEnumText(outcome),
			requestDamage,
			mitigatedDamage,
			finalTakenDamage,
			InPacket.CommittedDamage,
			InPacket.bDamageCommitted ? TEXT("true") : TEXT("false"))
		: FString::Printf(
			TEXT("ResultFrom: %s | ResultReceiver: %s | Outcome: %s | Commit: %.3f | DamageCommitted: %s"),
			*resultSourceName,
			*GetDisplayNameOrNA(InReceiverActor),
			*CompactStoreEnumText(outcome),
			InPacket.CommittedDamage,
			InPacket.bDamageCommitted ? TEXT("true") : TEXT("false"));

	store->Snapshot.LastCombat.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastCombat.FrameNumber = GetCurrentFrameNumber();
	store->Snapshot.LastCombat.WorldTimeSeconds = GetWorldTimeSeconds(world);
	store->Snapshot.LastCombat.SourceName = sourceName;
	store->Snapshot.LastCombat.TargetName = targetName;
	store->Snapshot.LastCombat.DamageCauserName = causerName;
	store->Snapshot.LastCombat.DefenseOutcome = outcome;
	store->Snapshot.LastCombat.bHasDamageCommit = true;
	store->Snapshot.LastCombat.bDamageCommitted = InPacket.bDamageCommitted;
	store->Snapshot.LastCombat.bHasDamageBreakdown = bHasPreviousDamageBreakdown;
	store->Snapshot.LastCombat.RequestDamage = requestDamage;
	store->Snapshot.LastCombat.MitigatedDamage = mitigatedDamage;
	store->Snapshot.LastCombat.FinalTakenDamage = finalTakenDamage;
	store->Snapshot.LastCombat.CommittedDamage = InPacket.CommittedDamage;
	store->Snapshot.LastCombat.Summary = summary;
	RecordRecentCombatPairInternal(*store, world, InPacket.SourceActor, InPacket.TargetActor, eventName);

	AddEventInternal(*store, MakeEventEntry(world, TEXT("CombatResult"), eventName, receiverName, sourceName, targetName, summary));
#endif
}

// AI Record

void FDebugOverlaySnapshotStore::RecordAICombatTask(const UObject* InWorldContextObject, const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const FString& InIntentState, const FString& InSubState, const FString& InRequestResult, const FString& InRejectReason, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	FDebugOverlayWorldStore* store = FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = ResolveWorld(InWorldContextObject);
	const FString eventName = ToSafeEventName(InEventName, TEXT("AICombatTask"));
	const FString controllerName = GetNameSafe(InAIController);
	const FString pawnName = GetNameSafe(InOwnerPawn);
	const FString targetName = GetNameSafe(InTargetActor);
	const FString summary = FString::Printf(
		TEXT("Controller: %s | Pawn: %s | Target: %s | IntentState: %s | SubState: %s | Result: %s | RejectReason: %s"),
		*GetDisplayNameOrNA(InAIController),
		*GetDisplayNameOrNA(InOwnerPawn),
		*GetDisplayNameOrNA(InTargetActor),
		*CompactStoreEnumText(InIntentState),
		*CompactStoreEnumText(InSubState),
		*CompactStoreEnumText(InRequestResult),
		*CompactReasonText(InRejectReason));

	store->Snapshot.LastAI.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastAI.FrameNumber = GetCurrentFrameNumber();
	store->Snapshot.LastAI.WorldTimeSeconds = GetWorldTimeSeconds(world);
	store->Snapshot.LastAI.ControllerName = controllerName;
	store->Snapshot.LastAI.PawnName = pawnName;
	store->Snapshot.LastAI.TargetName = targetName;
	store->Snapshot.LastAI.IntentState = InIntentState;
	store->Snapshot.LastAI.SubState = InSubState;
	store->Snapshot.LastAI.RequestResult = InRequestResult;
	store->Snapshot.LastAI.RejectReason = InRejectReason;
	store->Snapshot.LastAI.Summary = summary;

	AddEventInternal(*store, MakeEventEntry(world, TEXT("AI"), eventName, pawnName, pawnName, targetName, summary));
#endif
}

// Event Log

void FDebugOverlaySnapshotStore::AddEvent(const UObject* InWorldContextObject, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	FDebugOverlayWorldStore* store = FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = ResolveWorld(InWorldContextObject);
	AddEventInternal(*store, MakeEventEntry(world, InCategory, InEventName, InOwnerName, InSourceName, InTargetName, InSummary));
#endif
}

TArray<FDebugOverlayEventEntry> FDebugOverlaySnapshotStore::GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents)
{
#if !UE_BUILD_SHIPPING
	const FDebugOverlayWorldStore* store = FindStore(InWorldContextObject);
	if (!store) return TArray<FDebugOverlayEventEntry>();

	return GetRecentEventsCopyFromStore(*store, InMaxEvents, DebugOverlayEventStoreCapacity, TEXT("All"), true);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

TArray<FDebugOverlayEventEntry> FDebugOverlaySnapshotStore::GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter)
{
#if !UE_BUILD_SHIPPING
	const FDebugOverlayWorldStore* store = FindStore(InWorldContextObject);
	if (!store) return TArray<FDebugOverlayEventEntry>();

	return GetRecentEventsCopyFromStore(*store, InMaxEvents, DebugOverlayEventStoreCapacity, InFilter, true);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

TArray<FDebugOverlayEventEntry> FDebugOverlaySnapshotStore::GetRecentEventsForSubjectCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter, const FString& InSubjectName)
{
#if !UE_BUILD_SHIPPING
	const FDebugOverlayWorldStore* store = FindStore(InWorldContextObject);
	if (!store) return TArray<FDebugOverlayEventEntry>();

	return GetRecentEventsForSubjectCopyFromStore(*store, InMaxEvents, DebugOverlayEventStoreCapacity, InFilter, InSubjectName, true);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

// Snapshot Query

bool FDebugOverlaySnapshotStore::TryGetSnapshotCopy(const UObject* InWorldContextObject, FDebugOverlaySnapshot& OutSnapshot)
{
	OutSnapshot = FDebugOverlaySnapshot();

#if !UE_BUILD_SHIPPING
	const FDebugOverlayWorldStore* store = FindStore(InWorldContextObject);
	if (!store) return false;

	OutSnapshot = store->Snapshot;
	OutSnapshot.RecentEvents = GetRecentEventsCopyFromStore(*store, GetClampedEventLogDisplayLimit(), DebugOverlayMaxEventLogDisplayLimit, TEXT("All"), false);
	return true;
#else
	return false;
#endif
}

bool FDebugOverlaySnapshotStore::TryGetRecentCombatPair(const UObject* InWorldContextObject, FDebugOverlayRecentCombatPair& OutPair)
{
	OutPair = FDebugOverlayRecentCombatPair();

#if !UE_BUILD_SHIPPING
	const FDebugOverlayWorldStore* store = FindStore(InWorldContextObject);
	if (!store || !store->bHasRecentCombatPair) return false;

	OutPair = store->RecentCombatPair;
	return true;
#else
	return false;
#endif
}

// Lifecycle

void FDebugOverlaySnapshotStore::Reset(const UObject* InWorldContextObject)
{
#if !UE_BUILD_SHIPPING
	UWorld* world = ResolveWorld(InWorldContextObject);
	if (!IsValid(world)) return;

	StoresByWorld.Remove(TObjectKey<UWorld>(world));
#endif
}

void FDebugOverlaySnapshotStore::ResetAll()
{
#if !UE_BUILD_SHIPPING
	StoresByWorld.Reset();
#endif
}
