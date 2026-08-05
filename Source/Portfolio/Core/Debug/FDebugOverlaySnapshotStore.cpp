#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlaySnapshotStoreInternals.h"

#include "Type/CCombatResultTypes.h"
#include "Type/CCombatSignalTargetTypes.h"

#include "AIController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

bool FDebugOverlaySnapshotStore::IsEnabled()
{
#if !UE_BUILD_SHIPPING
	return SnapshotStoreConfig::IsEnabled();
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

void FDebugOverlaySnapshotStore::RecordExecutionDecision(const UObject* InWorldContextObject, const AActor* InOwnerActor, const FString& InDomain, const FString& InSubject, const FString& InDecision, const FString& InApplyMode, const FString& InRejectReason, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = SnapshotRecordBuilders::MakeSnapshotStamp(world);
	const FString eventName = SnapshotRecordBuilders::ToSafeEventName(InEventName, TEXT("ExecutionDecision"));
	const FString ownerName = GetNameSafe(InOwnerActor);
	const FString summary = FString::Printf(
		TEXT("Owner: %s | Domain: %s | Subject: %s | Decision: %s | Apply: %s | RejectReason: %s"),
		*SnapshotRecordBuilders::GetDisplayNameOrNA(InOwnerActor),
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(InDomain),
		InSubject.IsEmpty() ? TEXT("N/A") : *InSubject,
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(InDecision),
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(InApplyMode),
		*SnapshotRecordBuilders::NormalizeReasonDisplayText(InRejectReason));

	store->Snapshot.LastExecution.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastExecution.FrameNumber = stamp.FrameNumber;
	store->Snapshot.LastExecution.WorldTimeSeconds = stamp.WorldTimeSeconds;
	store->Snapshot.LastExecution.OwnerName = ownerName;
	store->Snapshot.LastExecution.Domain = InDomain;
	store->Snapshot.LastExecution.Decision = InDecision;
	store->Snapshot.LastExecution.ApplyMode = InApplyMode;
	store->Snapshot.LastExecution.RejectReason = InRejectReason;
	store->Snapshot.LastExecution.Summary = summary;

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, TEXT("Execution"), eventName, ownerName, FString(), FString(), summary));
#endif
}

void FDebugOverlaySnapshotStore::RecordWeaponCollisionWindow(const UObject* InWorldContextObject, const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, const FString& InHitWindowState, const TCHAR* InEventName, const TCHAR* InReason)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const FString eventName = SnapshotRecordBuilders::ToSafeEventName(InEventName, TEXT("WeaponCollisionWindow"));
	const FString ownerName = GetNameSafe(InOwnerActor);
	const FString summary = FString::Printf(
		TEXT("State: %s | HitWindow: %d | Collision: %s | Reason: %s"),
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(InHitWindowState),
		InHitWindowId,
		*InCollisionName.ToString(),
		*SnapshotRecordBuilders::NormalizeReasonDisplayText(SnapshotRecordBuilders::ToReasonOrNone(InReason)));

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, TEXT("Combat"), eventName, ownerName, ownerName, FString(), summary));
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
	const FString eventName = SnapshotRecordBuilders::ToSafeEventName(InEventName, TEXT("CombatTargetPacket"));
	const FString sourceName = GetNameSafe(InPacket.Context.SourceActor);
	const FString targetName = GetNameSafe(InPacket.Context.TargetActor);
	const FString causerName = GetNameSafe(InPacket.Context.DamageCauser);
	const FString outcome = UEnum::GetValueAsString(InPacket.Result.DefenseOutcome);
	const FString summary = FString::Printf(
		TEXT("Attacker: %s | Defender: %s | Outcome: %s | Request: %.3f | Mitigated: %.3f | Final: %.3f | Commit: %.3f | Accepted: %s"),
		*SnapshotRecordBuilders::GetDisplayNameOrNA(InPacket.Context.SourceActor),
		*SnapshotRecordBuilders::GetDisplayNameOrNA(InPacket.Context.TargetActor),
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(outcome),
		InPacket.Result.RequestDamage,
		InPacket.Result.MitigatedDamage,
		InPacket.Result.FinalTakenDamage,
		InPacket.Result.CommittedDamage,
		InPacket.Result.bAccepted ? TEXT("true") : TEXT("false"));

	store->Snapshot.LastCombat.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastCombat.FrameNumber = stamp.FrameNumber;
	store->Snapshot.LastCombat.WorldTimeSeconds = stamp.WorldTimeSeconds;
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

	SnapshotRecordBuilders::UpdateRecentCombatPair(*store, world, InPacket.Context.SourceActor, InPacket.Context.TargetActor, eventName);

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, TEXT("Combat"), eventName, targetName, sourceName, targetName, summary));
#endif
}

void FDebugOverlaySnapshotStore::RecordCombatResult(const UObject* InWorldContextObject, const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = SnapshotRecordBuilders::MakeSnapshotStamp(world);
	const FString eventName = SnapshotRecordBuilders::ToSafeEventName(InEventName, TEXT("CombatResult"));
	const FString receiverName = GetNameSafe(InReceiverActor);
	const FString sourceName = GetNameSafe(InPacket.SourceActor);
	const FString targetName = GetNameSafe(InPacket.TargetActor);
	const FString causerName = GetNameSafe(InPacket.DamageCauser);
	const FString outcome = UEnum::GetValueAsString(InPacket.DefenseOutcome);
	const FString resultSourceName = SnapshotRecordBuilders::ResolveCombatResultSourceName(InReceiverActor, InPacket);
	const bool bHasPreviousDamageBreakdown = store->Snapshot.LastCombat.bHasDamageBreakdown
		&& SnapshotRecordBuilders::IsSameCombatPair(store->Snapshot.LastCombat, sourceName, targetName);
	const float requestDamage = bHasPreviousDamageBreakdown ? store->Snapshot.LastCombat.RequestDamage : 0.f;
	const float mitigatedDamage = bHasPreviousDamageBreakdown ? store->Snapshot.LastCombat.MitigatedDamage : 0.f;
	const float finalTakenDamage = bHasPreviousDamageBreakdown ? store->Snapshot.LastCombat.FinalTakenDamage : 0.f;
	const FString summary = bHasPreviousDamageBreakdown
		? FString::Printf(
			TEXT("ResultFrom: %s | ResultReceiver: %s | Outcome: %s | Request: %.3f | Mitigated: %.3f | Final: %.3f | Commit: %.3f | DamageCommitted: %s"),
			*resultSourceName,
			*SnapshotRecordBuilders::GetDisplayNameOrNA(InReceiverActor),
			*SnapshotRecordBuilders::NormalizeEnumDisplayText(outcome),
			requestDamage,
			mitigatedDamage,
			finalTakenDamage,
			InPacket.CommittedDamage,
			InPacket.bDamageCommitted ? TEXT("true") : TEXT("false"))
		: FString::Printf(
			TEXT("ResultFrom: %s | ResultReceiver: %s | Outcome: %s | Commit: %.3f | DamageCommitted: %s"),
			*resultSourceName,
			*SnapshotRecordBuilders::GetDisplayNameOrNA(InReceiverActor),
			*SnapshotRecordBuilders::NormalizeEnumDisplayText(outcome),
			InPacket.CommittedDamage,
			InPacket.bDamageCommitted ? TEXT("true") : TEXT("false"));

	store->Snapshot.LastCombat.CaptureState = EDebugOverlayCaptureState::Captured;
	store->Snapshot.LastCombat.FrameNumber = stamp.FrameNumber;
	store->Snapshot.LastCombat.WorldTimeSeconds = stamp.WorldTimeSeconds;
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

	SnapshotRecordBuilders::UpdateRecentCombatPair(*store, world, InPacket.SourceActor, InPacket.TargetActor, eventName);

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, TEXT("CombatResult"), eventName, receiverName, sourceName, targetName, summary));
#endif
}

void FDebugOverlaySnapshotStore::RecordAICombatTask(const UObject* InWorldContextObject, const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const FString& InIntentState, const FString& InSubState, const FString& InRequestResult, const FString& InRejectReason, const TCHAR* InEventName)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	const DebugOverlaySnapshotStoreInternals::FDebugOverlaySnapshotStamp stamp = SnapshotRecordBuilders::MakeSnapshotStamp(world);
	const FString eventName = SnapshotRecordBuilders::ToSafeEventName(InEventName, TEXT("AICombatTask"));
	const FString controllerName = GetNameSafe(InAIController);
	const FString pawnName = GetNameSafe(InOwnerPawn);
	const FString targetName = GetNameSafe(InTargetActor);
	const FString summary = FString::Printf(
		TEXT("Controller: %s | Pawn: %s | Target: %s | IntentState: %s | SubState: %s | Result: %s | RejectReason: %s"),
		*SnapshotRecordBuilders::GetDisplayNameOrNA(InAIController),
		*SnapshotRecordBuilders::GetDisplayNameOrNA(InOwnerPawn),
		*SnapshotRecordBuilders::GetDisplayNameOrNA(InTargetActor),
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(InIntentState),
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(InSubState),
		*SnapshotRecordBuilders::NormalizeEnumDisplayText(InRequestResult),
		*SnapshotRecordBuilders::NormalizeReasonDisplayText(InRejectReason));

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
	if (!pawnName.IsEmpty())
	{
		store->Snapshot.LastAIByPawnName.FindOrAdd(pawnName) = store->Snapshot.LastAI;
	}

	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, TEXT("AI"), eventName, pawnName, pawnName, targetName, summary));
#endif
}

void FDebugOverlaySnapshotStore::AddEvent(const UObject* InWorldContextObject, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary)
{
#if !UE_BUILD_SHIPPING
	if (!IsCollecting()) return;

	DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindOrAddStore(InWorldContextObject);
	if (!store) return;

	const UWorld* world = StoreLifecycle::ResolveWorld(InWorldContextObject);
	EventRingAccess::AddEventInternal(*store, SnapshotRecordBuilders::MakeEventEntry(world, InCategory, InEventName, InOwnerName, InSourceName, InTargetName, InSummary));
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

TArray<FDebugOverlayEventEntry> FDebugOverlaySnapshotStore::GetRecentEventsForSubjectCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter, const FString& InSubjectName)
{
#if !UE_BUILD_SHIPPING
	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return TArray<FDebugOverlayEventEntry>();

	return EventRingAccess::GetRecentEventsForSubjectCopyFromStore(*store, InMaxEvents, DebugOverlaySnapshotStoreInternals::EventStoreCapacity, InFilter, InSubjectName, true);
#else
	return TArray<FDebugOverlayEventEntry>();
#endif
}

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

bool FDebugOverlaySnapshotStore::TryGetRecentAIForPawn(const UObject* InWorldContextObject, const FString& InPawnName, FDebugOverlayAISummary& OutSummary)
{
	OutSummary = FDebugOverlayAISummary();

#if !UE_BUILD_SHIPPING
	if (InPawnName.IsEmpty()) return false;

	const DebugOverlaySnapshotStoreInternals::FDebugOverlayWorldStore* store = StoreLifecycle::FindStore(InWorldContextObject);
	if (!store) return false;

	const FDebugOverlayAISummary* summary = store->Snapshot.LastAIByPawnName.Find(InPawnName);
	if (!summary) return false;

	OutSummary = *summary;
	return true;
#else
	return false;
#endif
}

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
