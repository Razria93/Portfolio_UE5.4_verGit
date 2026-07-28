#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlaySnapshotTypes.h"

class AAIController;
class APawn;
struct FCombatResultPacket;
struct FCombatSignalTargetPacket;

class PORTFOLIO_API FDebugOverlaySnapshotStore
{
public:
	static bool IsEnabled();
	static bool IsCollecting();
	static int32 GetEventLogDisplayLimit();

public:
	static void RecordExecutionDecision(const UObject* InWorldContextObject, const AActor* InOwnerActor, const FString& InDomain, const FString& InDecision, const FString& InApplyMode, const FString& InRejectReason, const TCHAR* InEventName);
	static void RecordWeaponCollisionWindow(const UObject* InWorldContextObject, const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, const FString& InHitWindowState, const TCHAR* InEventName, const TCHAR* InReason = nullptr);
	static void RecordCombatTargetPacket(const UObject* InWorldContextObject, const FCombatSignalTargetPacket& InPacket, const TCHAR* InEventName);
	static void RecordCombatResult(const UObject* InWorldContextObject, const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InEventName);
	static void RecordAICombatTask(const UObject* InWorldContextObject, const AAIController* InAIController, const APawn* InOwnerPawn, const AActor* InTargetActor, const FString& InIntent, const FString& InRequestResult, const FString& InRejectReason, const TCHAR* InEventName);
	static void AddEvent(const UObject* InWorldContextObject, const FString& InCategory, const FString& InEventName, const FString& InOwnerName, const FString& InSourceName, const FString& InTargetName, const FString& InSummary);

public:
	static bool GetSnapshotCopy(const UObject* InWorldContextObject, FDebugOverlaySnapshot& OutSnapshot);
	static TArray<FDebugOverlayEventEntry> GetRecentEventsCopy(const UObject* InWorldContextObject, int32 InMaxEvents);
	static void Reset(const UObject* InWorldContextObject);
	static void ResetAll();
};
