#include "Core/Debug/FDebugOverlaySnapshotStore.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "UObject/ObjectKey.h"

namespace
{
	static constexpr int32 DebugOverlayEventStoreCapacity = 32;
	static constexpr int32 DebugOverlayDefaultEventLogDisplayLimit = 5;
	static constexpr int32 DebugOverlayMaxEventLogDisplayLimit = 5;

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
		TEXT("Number of recent debug overlay event lines to display. 0-5."),
		ECVF_Default);
#endif

	struct FDebugOverlayWorldStore
	{
		FDebugOverlaySnapshot Snapshot;
		TArray<FDebugOverlayEventEntry> EventRing;
		int32 NextEventIndex = 0;
		int32 EventCount = 0;
	};

#if !UE_BUILD_SHIPPING
	TMap<TObjectKey<UWorld>, FDebugOverlayWorldStore> StoresByWorld;

	int32 GetClampedEventLogDisplayLimit();
	TArray<FDebugOverlayEventEntry> GetRecentEventsCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp);
#endif

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

#if !UE_BUILD_SHIPPING
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

		InStore.Snapshot.RecentEvents = GetRecentEventsCopyFromStore(InStore, GetClampedEventLogDisplayLimit(), DebugOverlayMaxEventLogDisplayLimit);
	}

	TArray<FDebugOverlayEventEntry> GetRecentEventsCopyFromStore(const FDebugOverlayWorldStore& InStore, int32 InMaxEvents, int32 InMaxClamp)
	{
		TArray<FDebugOverlayEventEntry> result;

		const int32 maxEvents = FMath::Clamp(InMaxEvents, 0, InMaxClamp);
		const int32 eventCount = FMath::Min(InStore.EventCount, maxEvents);
		result.Reserve(eventCount);

		for (int32 i = 0; i < eventCount; ++i)
		{
			const int32 index = (InStore.NextEventIndex - 1 - i + DebugOverlayEventStoreCapacity) % DebugOverlayEventStoreCapacity;
			if (InStore.EventRing.IsValidIndex(index))
			{
				result.Add(InStore.EventRing[index]);
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

// Execution Record

void FDebugOverlaySnapshotStore::RecordExecutionDecision(const UObject* InWorldContextObject, const AActor* InOwnerActor, const FString& InDomain, const FString& InDecision, const FString& InApplyMode, const FString& InRejectReason, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	FDebugOverlayWorldStore* store = FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = ResolveWorld(InWorldContextObject);
	const FString eventName = ToSafeEventName(InEventName, TEXT("ExecutionDecision"));
	const FString ownerName = GetNameSafe(InOwnerActor);
	const FString summary = FString::Printf(
		TEXT("%s %s Apply=%s Reject=%s"),
		*InDomain,
		*InDecision,
		*InApplyMode,
		*InRejectReason);

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
	const FString weaponName = GetNameSafe(InWeaponActor);
	const FString summary = FString::Printf(
		TEXT("%s HitWindow=%d Collision=%s Reason=%s"),
		*InHitWindowState,
		InHitWindowId,
		*InCollisionName.ToString(),
		*ToSafeReason(InReason));

	store->Snapshot.LastCombat.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastCombat.FrameNumber = GetCurrentFrameNumber();
	store->Snapshot.LastCombat.WorldTimeSeconds = GetWorldTimeSeconds(world);
	store->Snapshot.LastCombat.SourceName = ownerName;
	store->Snapshot.LastCombat.DamageCauserName = weaponName;
	store->Snapshot.LastCombat.HitWindowId = InHitWindowId;
	store->Snapshot.LastCombat.HitWindowState = InHitWindowState;
	store->Snapshot.LastCombat.Summary = summary;

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
		TEXT("Outcome=%s Final=%.3f Commit=%.3f Accepted=%s"),
		*outcome,
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
	store->Snapshot.LastCombat.FinalTakenDamage = InPacket.Result.FinalTakenDamage;
	store->Snapshot.LastCombat.CommittedDamage = InPacket.Result.CommittedDamage;
	store->Snapshot.LastCombat.Summary = summary;

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
	const FString summary = FString::Printf(
		TEXT("Outcome=%s DamageCommitted=%s Commit=%.3f Receiver=%s"),
		*outcome,
		InPacket.bDamageCommitted ? TEXT("true") : TEXT("false"),
		InPacket.CommittedDamage,
		*receiverName);

	store->Snapshot.LastCombat.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastCombat.FrameNumber = GetCurrentFrameNumber();
	store->Snapshot.LastCombat.WorldTimeSeconds = GetWorldTimeSeconds(world);
	store->Snapshot.LastCombat.SourceName = sourceName;
	store->Snapshot.LastCombat.TargetName = targetName;
	store->Snapshot.LastCombat.DamageCauserName = causerName;
	store->Snapshot.LastCombat.DefenseOutcome = outcome;
	store->Snapshot.LastCombat.bHasDamageCommit = true;
	store->Snapshot.LastCombat.bDamageCommitted = InPacket.bDamageCommitted;
	store->Snapshot.LastCombat.CommittedDamage = InPacket.CommittedDamage;
	store->Snapshot.LastCombat.Summary = summary;

	AddEventInternal(*store, MakeEventEntry(world, TEXT("CombatResult"), eventName, receiverName, sourceName, targetName, summary));
#endif
}

// AI Record

void FDebugOverlaySnapshotStore::RecordAICombatTask(const UObject* InWorldContextObject, const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const FString& InIntent, const FString& InRequestResult, const FString& InRejectReason, const TCHAR* InEventName)
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
		TEXT("Intent=%s Result=%s Reject=%s"),
		*InIntent,
		*InRequestResult,
		*InRejectReason);

	store->Snapshot.LastAI.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastAI.FrameNumber = GetCurrentFrameNumber();
	store->Snapshot.LastAI.WorldTimeSeconds = GetWorldTimeSeconds(world);
	store->Snapshot.LastAI.ControllerName = controllerName;
	store->Snapshot.LastAI.PawnName = pawnName;
	store->Snapshot.LastAI.TargetName = targetName;
	store->Snapshot.LastAI.Intent = InIntent;
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

	return GetRecentEventsCopyFromStore(*store, InMaxEvents, DebugOverlayEventStoreCapacity);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

// Snapshot Query

bool FDebugOverlaySnapshotStore::GetSnapshotCopy(const UObject* InWorldContextObject, FDebugOverlaySnapshot& OutSnapshot)
{
	OutSnapshot = FDebugOverlaySnapshot();

#if !UE_BUILD_SHIPPING
	const FDebugOverlayWorldStore* store = FindStore(InWorldContextObject);
	if (!store) return false;

	OutSnapshot = store->Snapshot;
	OutSnapshot.RecentEvents = GetRecentEventsCopyFromStore(*store, GetClampedEventLogDisplayLimit(), DebugOverlayMaxEventLogDisplayLimit);
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
