#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlaySnapshotTypes.h"

class AAIController;
class AActor;
class APawn;
struct FCombatResultPacket;
struct FCombatSignalTargetPacket;

struct PORTFOLIO_API FDebugOverlayRecentCombatPair
{
	TWeakObjectPtr<AActor> SourceActor;
	TWeakObjectPtr<AActor> TargetActor;
	FString SourceName;
	FString TargetName;
	uint64 FrameNumber = 0;
	float WorldTimeSeconds = 0.f;
	FString EventName;
};

class PORTFOLIO_API FDebugOverlaySnapshotStore
{
public:
	// Gate
	static bool IsEnabled();
	static bool IsCollecting();
	// Returns the display limit clamped to the internal accepted range.
	static int32 GetEventLogDisplayLimit();
	// Returns canonical filter text: All, Execution, Combat, or AI.
	static FString GetEventLogFilter();

public:
	// Execution Record
	static void RecordExecutionDecision(const UObject* InWorldContextObject, const AActor* InOwnerActor, const FString& InDomain, const FString& InSubject, const FString& InDecision, const FString& InApplyMode, const FString& InRejectReason, const TCHAR* InEventName);

public:
	// Combat Record
	static void RecordWeaponCollisionWindow(const UObject* InWorldContextObject, const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, const FString& InHitWindowState, const TCHAR* InEventName, const TCHAR* InReason = nullptr);
	static void RecordCombatTargetPacket(const UObject* InWorldContextObject, const FCombatSignalTargetPacket& InPacket, const TCHAR* InEventName);
	static void RecordCombatResult(const UObject* InWorldContextObject, const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InEventName);

public:
	// AI Record
	static void RecordAICombatTask(const UObject* InWorldContextObject, const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const FString& InIntentState, const FString& InSubState, const FString& InRequestResult, const FString& InRejectReason, const TCHAR* InEventName);

public:
	// Event Log
	static void AddEvent(const UObject* InWorldContextObject, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary);
	// Returns newest-first entries and applies display-level suppression filters.
	static TArray<FDebugOverlayEventEntry> GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents);
	// Same as above with category filter (InFilter is normalized internally).
	static TArray<FDebugOverlayEventEntry> GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter);
	// Returns newest-first entries scoped to a subject actor name and role-matched categories.
	static TArray<FDebugOverlayEventEntry> GetRecentEventsForSubjectCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter, const FString& InSubjectName);

public:
	// Snapshot Query
	// Success: world store exists for InWorldContextObject and OutSnapshot receives a full copy.
	// Failure: OutSnapshot is reset to default and function returns false.
	static bool TryGetSnapshotCopy(const UObject* InWorldContextObject, FDebugOverlaySnapshot& OutSnapshot);
	// Success: world store exists and a recent combat pair was recorded.
	// Failure: OutPair is reset to default and function returns false.
	static bool TryGetRecentCombatPair(const UObject* InWorldContextObject, FDebugOverlayRecentCombatPair& OutPair);
	// Success: InPawnName is not empty, world store exists, and the pawn cache has an entry.
	// Failure: OutSummary is reset to default and function returns false.
	static bool TryGetRecentAIForPawn(const UObject* InWorldContextObject, const FString& InPawnName, FDebugOverlayAISummary& OutSummary);

public:
	// Lifecycle
	static void Reset(const UObject* InWorldContextObject);
	static void ResetAll();
};
