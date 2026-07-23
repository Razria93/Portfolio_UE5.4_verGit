#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatSignalTypes.h"
#include "Type/CWeaponStructure.h"

class PORTFOLIO_API FCombatSignalDebug
{
public:
	// Gate
	static bool ShouldAuditCombatSignal();
	static bool ShouldPrintCombatSignalDebug();

public:
	// Weapon Actor Diagnostic Hook
	static void RecordWeaponCollisionWindowForAudit(const AActor* InOwnerActor, const AActor* InWeaponActor, FName InCollisionName, int32 InHitWindowId, int32 InCollisionCount, const TCHAR* InEvent, const TCHAR* InReason = nullptr);
	static void RecordWeaponOverlapAcceptedForAudit(const FHitContext& InHitContext, const TCHAR* InEvent);
	static void RecordWeaponOverlapRejectedForAudit(const AActor* InOwnerActor, const AActor* InWeaponActor, const UPrimitiveComponent* InOverlappedComponent, const AActor* InOtherActor, const UPrimitiveComponent* InOtherComponent, int32 InHitWindowId, const TCHAR* InEvent, const TCHAR* InReason);
	static void RecordWeaponOverlapIgnoredForAudit(const AActor* InOwnerActor, const AActor* InWeaponActor, const UPrimitiveComponent* InOverlappedComponent, const AActor* InOtherActor, const UPrimitiveComponent* InOtherComponent, int32 InHitWindowId, const TCHAR* InEvent, const TCHAR* InReason);

public:
	// Weapon Actor Debug Dump
	static void PrintWeaponHitContextDebug(const FHitContext& InHitContext);

public:
	// Source Diagnostic Hook
	static void RecordSourceHitRequestRejectedForAudit(const FHitContext& InHitContext, const TCHAR* InReason);

	static void RecordSourceAcceptedForAudit(const FCombatSignalSourceContext& InContext);
	static void RecordSourceRejectedForAudit(const FCombatSignalSourceContext& InContext);

	static void RecordCueAcceptedForAudit(const FCombatSignal& InSignal);
	static void RecordCueRejectedForAudit(const FCombatSignal& InSignal, const TCHAR* InReason);

public:
	// Source Debug Dump
	static void PrintSourceContextDebug(const FCombatSignalSourceContext& InContext);

public:
	// Target Diagnostic Hook
	static void RecordTargetDamageRequestRejectedForAudit(float InDamageAmount, const FDamageEvent& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser, const TCHAR* InReason);

	static void RecordTargetAcceptedForAudit(const FCombatSignalTargetPacket& InPacket);
	static void RecordTargetRejectedForAudit(const FCombatSignalTargetPacket& InPacket);

	static void RecordTimingCueAcceptedForAudit(const FCombatSignal& InSignal);
	static void RecordTimingCueRejectedForAudit(const FCombatSignal& InSignal, const TCHAR* InReason);

public:
	// Target Debug Dump
	static void PrintTargetPacketDebug(const FCombatSignalTargetPacket& InPacket);

public:
	// Shared Dispatch Diagnostic Hook
	static void RecordCombatResultDispatchForAudit(const FCombatSignalTargetPacket& InTargetPacket, const FCombatResultPacket& InResultPacket, const AActor* InReceiverActor, const TCHAR* InEvent);
};
