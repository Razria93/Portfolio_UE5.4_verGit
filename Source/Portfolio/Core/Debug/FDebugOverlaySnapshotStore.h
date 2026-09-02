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
	// ===== Runtime Gates =====

	static bool IsEnabled();
	static bool IsCollecting();
	static int32 GetEventLogDisplayLimit();
	static FString GetEventLogFilter();
	static FString GetEventLogScope();

	// ===== Execution Record API =====

	static void RecordExecutionDecision(const UObject* InWorldContextObject, const AActor* InOwnerActor, const FString& InDomain, const FString& InSubject, const FString& InDecision, const FString& InApplyMode, const FString& InRejectReason, const TCHAR* InEventName);

	// ===== Combat Record API =====

	static void RecordWeaponCollisionWindow(const UObject* InWorldContextObject, const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, const FString& InHitWindowState, const TCHAR* InEventName, const TCHAR* InReason = nullptr);
	static void RecordCombatTargetPacket(const UObject* InWorldContextObject, const FCombatSignalTargetPacket& InPacket, const TCHAR* InEventName);
	static void RecordCombatResult(const UObject* InWorldContextObject, const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InEventName);

	// ===== AI Record API =====

	static void RecordAICombatTask(const UObject* InWorldContextObject, const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const FString& InIntentState, const FString& InSubState, const FString& InRequestResult, const FString& InRejectReason, const TCHAR* InEventName);

	// ===== Facing Record API =====

	static void RecordFacingTransition(const UObject* InWorldContextObject, const FDebugOverlayFacingTransition& InTransition);

	// ===== Event Log API =====

	static void AddEvent(const UObject* InWorldContextObject, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary, const AActor* InOwnerActor = nullptr, const AActor* InSourceActor = nullptr, const AActor* InTargetActor = nullptr);
	static TArray<FDebugOverlayEventEntry> GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents);
	static TArray<FDebugOverlayEventEntry> GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter);
	static TArray<FDebugOverlayEventEntry> GetRecentEventsForActorCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter, const AActor* InActor);
	static TArray<FDebugOverlayEventEntry> GetRecentEventsForSubjectCopy(const UObject* InWorldContextObject, int32 InMaxEvents, const FString& InFilter, const FString& InSubjectName);
	static void RemoveActorEventHistory(const UObject* InWorldContextObject, const AActor* InActor);

	// ===== Snapshot Query API =====

	static bool TryGetSnapshotCopy(const UObject* InWorldContextObject, FDebugOverlaySnapshot& OutSnapshot);
	static bool TryGetRecentCombatPair(const UObject* InWorldContextObject, FDebugOverlayRecentCombatPair& OutPair);
	static bool TryGetRecentAIForPawn(const UObject* InWorldContextObject, const FString& InPawnName, FDebugOverlayAISummary& OutSummary);
	static bool TryGetRecentFacingForPawn(const UObject* InWorldContextObject, const FString& InPawnName, FDebugOverlayFacingSummary& OutSummary);

	// ===== Lifecycle API =====

	static void Reset(const UObject* InWorldContextObject);
	static void ResetAll();
};
