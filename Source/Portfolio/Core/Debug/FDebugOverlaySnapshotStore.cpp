#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"
#include "Core/Debug/FDebugOverlayEventCategory.h"

#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalTargetTypes.h"

#include "AIController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

#if !UE_BUILD_SHIPPING
namespace
{
	void PruneInvalidAIActorSummaries(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore)
	{
		for (auto it = InStore.LastAIByPawn.CreateIterator(); it; ++it)
		{
			if (!it.Key().IsValid())
			{
				it.RemoveCurrent();
			}
		}
	}

	void EnforceAIActorSummaryCapacity(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore)
	{
		while (InStore.LastAIByPawn.Num() >= DebugOverlaySnapshotStoreInternals::MaxAIActorSummaries)
		{
			TOptional<TWeakObjectPtr<APawn>> oldestKey;
			uint64 oldestFrameNumber = MAX_uint64;
			for (const TPair<TWeakObjectPtr<APawn>, FDebugOverlayAISummary>& pair : InStore.LastAIByPawn)
			{
				if (pair.Value.FrameNumber < oldestFrameNumber)
				{
					oldestKey = pair.Key;
					oldestFrameNumber = pair.Value.FrameNumber;
				}
			}

			if (!oldestKey.IsSet()) return;
			InStore.LastAIByPawn.Remove(oldestKey.GetValue());
		}
	}

	void RecordAIActorSummary(DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore& InStore, const APawn* InPawn, const FDebugOverlayAISummary& InSummary)
	{
		if (!IsValid(InPawn)) return;

		const TWeakObjectPtr<APawn> pawnKey(const_cast<APawn*>(InPawn));
		if (!InStore.LastAIByPawn.Contains(pawnKey))
		{
			PruneInvalidAIActorSummaries(InStore);
			EnforceAIActorSummaryCapacity(InStore);
		}

		InStore.LastAIByPawn.FindOrAdd(pawnKey) = InSummary;
	}
}
#endif

// ===== Runtime Gates =====

bool FDebugOverlaySnapshotStore::IsHudVisible()
{
#if !UE_BUILD_SHIPPING
	return SnapshotStoreConfig::IsHudVisible();
#else
	return false;
#endif
}

bool FDebugOverlaySnapshotStore::IsCollecting()
{
#if !UE_BUILD_SHIPPING
	return SnapshotStoreConfig::IsCollecting();
#else
	return false;
#endif
}

int32 FDebugOverlaySnapshotStore::GetEventLogDisplayLimit()
{
#if !UE_BUILD_SHIPPING
	return EventFilterPolicy::GetClampedEventLogDisplayLimit();
#else
	return 0;
#endif
}

FString FDebugOverlaySnapshotStore::GetEventLogFilter()
{
#if !UE_BUILD_SHIPPING
	return EventFilterPolicy::GetCanonicalEventLogFilter();
#else
	return TEXT("All");
#endif
}

FString FDebugOverlaySnapshotStore::GetEventLogScope()
{
#if !UE_BUILD_SHIPPING
	return EventFilterPolicy::GetCanonicalEventLogScope();
#else
	return TEXT("World");
#endif
}

// ===== Execution Record API =====

void FDebugOverlaySnapshotStore::RecordExecutionDecision(const UObject* InWorldContextObject, const AActor* InOwnerActor, const FString& InDomain, const FString& InSubject, const FString& InDecision, const FString& InApplyMode, const FString& InRejectReason, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = SnapshotRecordBuilders::MakeSnapshotStamp(world);
	const FString eventName = SnapshotRecordBuilders::FormatEventNameOrFallback(InEventName, TEXT("ActionReactionDecision"));
	const FString ownerName = GetNameSafe(InOwnerActor);
	const FString summary = FString::Printf(
		TEXT("Owner: %s | Domain: %s | Subject: %s | Decision: %s | Apply: %s | RejectReason: %s"),
		*SnapshotRecordBuilders::FormatDisplayNameOrNA(InOwnerActor),
		*SnapshotRecordBuilders::FormatCompactEnumText(InDomain),
		InSubject.IsEmpty() ? TEXT("N/A") : *InSubject,
		*SnapshotRecordBuilders::FormatCompactEnumText(InDecision),
		*SnapshotRecordBuilders::FormatCompactEnumText(InApplyMode),
		*SnapshotRecordBuilders::FormatCompactReasonText(InRejectReason));

	store->Snapshot.LastActionReaction.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastActionReaction.FrameNumber = stamp.FrameNumber;
	store->Snapshot.LastActionReaction.WorldTimeSeconds = stamp.WorldTimeSeconds;
	store->Snapshot.LastActionReaction.OwnerName = ownerName;
	store->Snapshot.LastActionReaction.Domain = InDomain;
	store->Snapshot.LastActionReaction.Decision = InDecision;
	store->Snapshot.LastActionReaction.ApplyMode = InApplyMode;
	store->Snapshot.LastActionReaction.RejectReason = InRejectReason;
	store->Snapshot.LastActionReaction.Summary = summary;

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, DebugOverlayEventCategory::ActionReaction, eventName, ownerName, FString(), FString(), summary), InOwnerActor);
#endif
}

// ===== Combat Record API =====

void FDebugOverlaySnapshotStore::RecordWeaponCollisionWindow(const UObject* InWorldContextObject, const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, const FString& InHitWindowState, const TCHAR* InEventName, const TCHAR* InReason)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const FString eventName = SnapshotRecordBuilders::FormatEventNameOrFallback(InEventName, TEXT("WeaponCollisionWindow"));
	const FString ownerName = GetNameSafe(InOwnerActor);

	FDebugOverlayCombatEventDetails combatDetails;
	combatDetails.Kind = EDebugOverlayCombatEventKind::CollisionWindow;
	combatDetails.CollisionState = SnapshotRecordBuilders::FormatCompactEnumText(InHitWindowState);
	combatDetails.HitWindowId = InHitWindowId;
	combatDetails.CollisionName = InCollisionName.ToString();
	combatDetails.Reason = SnapshotRecordBuilders::FormatCompactReasonText(SnapshotRecordBuilders::FormatReasonOrNone(InReason));

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeCombatEventEntry(world, eventName, ownerName, ownerName, FString(), combatDetails), InOwnerActor, InWeaponActor);
#endif
}

void FDebugOverlaySnapshotStore::RecordCombatTargetPacket(const UObject* InWorldContextObject, const FCombatSignalTargetPacket& InPacket, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = SnapshotRecordBuilders::MakeSnapshotStamp(world);
	const FString eventName = SnapshotRecordBuilders::FormatEventNameOrFallback(InEventName, TEXT("CombatTargetPacket"));
	const FString sourceName = GetNameSafe(InPacket.Context.SourceActor);
	const FString targetName = GetNameSafe(InPacket.Context.TargetActor);
	const FString causerName = GetNameSafe(InPacket.Context.DamageCauser);
	const FString defenseOutcome = UEnum::GetValueAsString(InPacket.Result.DefenseOutcome);
	const FString reactionOutcome = UEnum::GetValueAsString(InPacket.Result.ReactionOutcome);

	FDebugOverlayCombatEventDetails combatDetails;
	combatDetails.Kind = EDebugOverlayCombatEventKind::TargetResolution;
	combatDetails.DefenseOutcome = defenseOutcome;
	combatDetails.ReactionOutcome = reactionOutcome;
	combatDetails.bHasDamageBreakdown = true;
	combatDetails.RequestDamage = InPacket.Result.RequestDamage;
	combatDetails.MitigatedDamage = InPacket.Result.MitigatedDamage;
	combatDetails.FinalTakenDamage = InPacket.Result.FinalTakenDamage;
	combatDetails.bHasDamageCommit = true;
	combatDetails.bDamageCommitted = !FMath::IsNearlyZero(InPacket.Result.CommittedDamage);
	combatDetails.CommittedDamage = InPacket.Result.CommittedDamage;
	combatDetails.bHasAccepted = true;
	combatDetails.bAccepted = InPacket.Result.bAccepted;

	FDebugOverlayCombatResolutionSummary& lastCombatResolution = store->Snapshot.LastCombatResolution;
	lastCombatResolution.CaptureState = EDebugOverlayCaptureState::Captured;
	lastCombatResolution.FrameNumber = stamp.FrameNumber;
	lastCombatResolution.WorldTimeSeconds = stamp.WorldTimeSeconds;
	lastCombatResolution.SourceName = sourceName;
	lastCombatResolution.TargetName = targetName;
	lastCombatResolution.DamageCauserName = causerName;
	lastCombatResolution.DefenseOutcome = defenseOutcome;
	lastCombatResolution.ReactionOutcome = reactionOutcome;
	lastCombatResolution.bHasDamageCommit = true;
	lastCombatResolution.bDamageCommitted = !FMath::IsNearlyZero(InPacket.Result.CommittedDamage);
	lastCombatResolution.bHasDamageBreakdown = true;
	lastCombatResolution.RequestDamage = InPacket.Result.RequestDamage;
	lastCombatResolution.MitigatedDamage = InPacket.Result.MitigatedDamage;
	lastCombatResolution.FinalTakenDamage = InPacket.Result.FinalTakenDamage;
	lastCombatResolution.CommittedDamage = InPacket.Result.CommittedDamage;

	SnapshotRecordBuilders::UpdateRecentCombatPair(*store, world, InPacket.Context.SourceActor, InPacket.Context.TargetActor, eventName);

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeCombatEventEntry(world, eventName, targetName, sourceName, targetName, combatDetails), InPacket.Context.TargetActor, InPacket.Context.SourceActor, InPacket.Context.TargetActor);
#endif
}

void FDebugOverlaySnapshotStore::RecordCombatResult(const UObject* InWorldContextObject, const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const FString eventName = SnapshotRecordBuilders::FormatEventNameOrFallback(InEventName, TEXT("CombatResult"));
	const FString receiverName = GetNameSafe(InReceiverActor);
	const FString sourceName = GetNameSafe(InPacket.SourceActor);
	const FString targetName = GetNameSafe(InPacket.TargetActor);
	const FString outcome = UEnum::GetValueAsString(InPacket.DefenseOutcome);

	FDebugOverlayCombatEventDetails combatDetails;
	combatDetails.Kind = EDebugOverlayCombatEventKind::ResultDelivery;
	combatDetails.DefenseOutcome = outcome;
	combatDetails.bHasDamageCommit = true;
	combatDetails.bDamageCommitted = InPacket.bDamageCommitted;
	combatDetails.CommittedDamage = InPacket.CommittedDamage;

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeCombatEventEntry(world, eventName, receiverName, sourceName, targetName, combatDetails), InReceiverActor, InPacket.SourceActor, InPacket.TargetActor);
#endif
}

// ===== AI Record API =====

void FDebugOverlaySnapshotStore::RecordAICombatTask(const UObject* InWorldContextObject, const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const FString& InIntentState, const FString& InSubState, const FString& InRequestResult, const FString& InRejectReason, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = SnapshotRecordBuilders::MakeSnapshotStamp(world);
	const FString eventName = SnapshotRecordBuilders::FormatEventNameOrFallback(InEventName, TEXT("AICombatTask"));
	const FString controllerName = GetNameSafe(InAIController);
	const FString pawnName = GetNameSafe(InOwnerPawn);
	const FString targetName = GetNameSafe(InTargetActor);
	const FString summary = FString::Printf(
		TEXT("Controller: %s | Pawn: %s | Target: %s | IntentState: %s | SubState: %s | Result: %s | RejectReason: %s"),
		*SnapshotRecordBuilders::FormatDisplayNameOrNA(InAIController),
		*SnapshotRecordBuilders::FormatDisplayNameOrNA(InOwnerPawn),
		*SnapshotRecordBuilders::FormatDisplayNameOrNA(InTargetActor),
		*SnapshotRecordBuilders::FormatCompactEnumText(InIntentState),
		*SnapshotRecordBuilders::FormatCompactEnumText(InSubState),
		*SnapshotRecordBuilders::FormatCompactEnumText(InRequestResult),
		*SnapshotRecordBuilders::FormatCompactReasonText(InRejectReason));

	store->Snapshot.LastAI.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastAI.FrameNumber = stamp.FrameNumber;
	store->Snapshot.LastAI.WorldTimeSeconds = stamp.WorldTimeSeconds;
	store->Snapshot.LastAI.ControllerName = controllerName;
	store->Snapshot.LastAI.PawnName = pawnName;
	store->Snapshot.LastAI.TargetName = targetName;
	store->Snapshot.LastAI.IntentState = InIntentState;
	store->Snapshot.LastAI.SubState = InSubState;
	store->Snapshot.LastAI.RequestResult = InRequestResult;
	store->Snapshot.LastAI.RejectReason = InRejectReason;
	store->Snapshot.LastAI.Summary = summary;
	RecordAIActorSummary(*store, InOwnerPawn, store->Snapshot.LastAI);

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, DebugOverlayEventCategory::AI, eventName, pawnName, pawnName, targetName, summary), InOwnerPawn, InOwnerPawn, InTargetActor);
#endif
}

// ===== Facing Record API =====

void FDebugOverlaySnapshotStore::RecordFacingTransition(const UObject* InWorldContextObject, const FDebugOverlayFacingTransition& InTransition)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	FDebugOverlayFacingTransition transition = InTransition;
	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = SnapshotRecordBuilders::MakeSnapshotStamp(world);
	transition.Current.CaptureState = EDebugOverlayCaptureState::Captured;
	transition.Current.FrameNumber = stamp.FrameNumber;
	transition.Current.WorldTimeSeconds = stamp.WorldTimeSeconds;

	const FString ownerName = transition.Current.OwnerName;
	if (ownerName.IsEmpty()) return;

	EventRingAccess::AddEventInternal(
		*store,
		SnapshotRecordBuilders::MakeEventEntry(
			world,
			DebugOverlayEventCategory::Facing,
			transition.Current.EventName,
			ownerName,
			transition.Current.BoundControllerName,
			transition.Current.CombatTargetName,
			transition.Current.Summary),
		transition.OwnerActor.Get(),
		nullptr,
		transition.CombatTargetActor.Get());
#endif
}

// ===== Event Log API =====

void FDebugOverlaySnapshotStore::AddEvent(const UObject* InWorldContextObject, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary, const AActor* InOwnerActor, const AActor* InSourceActor, const AActor* InTargetActor)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, InCategory, InEventName, InOwnerName, InSourceName, InTargetName, InSummary), InOwnerActor, InSourceActor, InTargetActor);
#endif
}

TArray<FDebugOverlayEventEntry> FDebugOverlaySnapshotStore::GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents)
{
#if !UE_BUILD_SHIPPING
	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return TArray<FDebugOverlayEventEntry>();

	return EventRingAccess::GetRecentEventsCopyFromStore(*store, InMaxEvents, DebugOverlaySnapshotStoreInternals::EventStoreCapacity, TEXT("All"), true);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

TArray<FDebugOverlayEventEntry> FDebugOverlaySnapshotStore::GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter)
{
#if !UE_BUILD_SHIPPING
	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return TArray<FDebugOverlayEventEntry>();

	return EventRingAccess::GetRecentEventsCopyFromStore(*store, InMaxEvents, DebugOverlaySnapshotStoreInternals::EventStoreCapacity, InFilter, true);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

// ===== Snapshot Query API =====

bool FDebugOverlaySnapshotStore::TryGetSnapshotCopy(const UObject* InWorldContextObject, FDebugOverlaySnapshot& OutSnapshot)
{
	OutSnapshot = FDebugOverlaySnapshot();

#if !UE_BUILD_SHIPPING
	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return false;

	OutSnapshot = store->Snapshot;
	OutSnapshot.RecentEvents = EventRingAccess::GetRecentEventsCopyFromStore(
		*store,
		EventFilterPolicy::GetClampedEventLogDisplayLimit(),
		DebugOverlaySnapshotStoreInternals::MaxEventLogDisplayLimit,
		TEXT("All"),
		false);
	return true;
#else
	return false;
#endif
}

bool FDebugOverlaySnapshotStore::TryGetRecentCombatPair(const UObject* InWorldContextObject, FDebugOverlayRecentCombatPair& OutPair)
{
	OutPair = FDebugOverlayRecentCombatPair();

#if !UE_BUILD_SHIPPING
	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store || !store->bHasRecentCombatPair) return false;

	OutPair = store->RecentCombatPair;
	return true;
#else
	return false;
#endif
}

bool FDebugOverlaySnapshotStore::TryGetRecentAIForActor(const UObject* InWorldContextObject, const APawn* InPawn, FDebugOverlayAISummary& OutSummary)
{
	OutSummary = FDebugOverlayAISummary();

#if !UE_BUILD_SHIPPING
	if (!IsValid(InPawn)) return false;

	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return false;

	const FDebugOverlayAISummary* summary = store->LastAIByPawn.Find(TWeakObjectPtr<APawn>(const_cast<APawn*>(InPawn)));
	if (!summary) return false;

	OutSummary = *summary;
	return true;
#else
	return false;
#endif
}

void FDebugOverlaySnapshotStore::RemoveActorDebugData(const UObject* InWorldContextObject, const AActor* InActor)
{
#if !UE_BUILD_SHIPPING
	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return;

	EventRingAccess::RemoveActorEventHistoryFromStore(*store, InActor);
	if (const APawn* pawn = Cast<APawn>(InActor))
	{
		store->LastAIByPawn.Remove(TWeakObjectPtr<APawn>(const_cast<APawn*>(pawn)));
	}
#endif
}

TArray<FDebugOverlayEventEntry> FDebugOverlaySnapshotStore::GetRecentEventsForActorCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter, const AActor* InActor)
{
#if !UE_BUILD_SHIPPING
	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return TArray<FDebugOverlayEventEntry>();

	return EventRingAccess::GetRecentEventsForActorCopyFromStore(*store, InMaxEvents, DebugOverlaySnapshotStoreInternals::EventStoreCapacity, InFilter, InActor, true);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

// ===== Lifecycle API =====

void FDebugOverlaySnapshotStore::Reset(const UObject* InWorldContextObject)
{
#if !UE_BUILD_SHIPPING
	UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	if (!IsValid(world)) return;

	StoreLifecycle::RemoveStoreForWorld(world);
#endif
}

void FDebugOverlaySnapshotStore::ResetAll()
{
#if !UE_BUILD_SHIPPING
	StoreLifecycle::ResetAllStores();
#endif
}
