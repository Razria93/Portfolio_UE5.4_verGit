#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatSignalStructure.h"
#include "Type/CWeaponStructure.h"

class PORTFOLIO_API FCombatSignalDebug
{
public:
	// Gate
	static bool ShouldAuditCombatSignal();
	static bool ShouldPrintCombatSignalDebug();

public:
	// Source Diagnostic Hook
	static void RecordSourceInvalidRequestForAudit(const FHitContext& InHitContext, const TCHAR* InReason);
	static void RecordSourceRejectedForAudit(const FCombatSignalSourceContext& InContext);
	static void RecordSourceAcceptedForAudit(const FCombatSignalSourceContext& InContext);

	static void RecordCueRejectedForAudit(const FCombatSignal& InSignal, const TCHAR* InReason);
	static void RecordCueAcceptedForAudit(const FCombatSignal& InSignal);

public:
	// Source Debug Dump
	static void PrintSourceContextDebug(const FCombatSignalSourceContext& InContext);

public:
	// Target Diagnostic Hook
	static void RecordTargetInvalidDamageRequestForAudit(float InDamageAmount, const FDamageEvent& InDamageEvent, AController* InEventInstigator, AActor* InDamageCauser, const TCHAR* InReason);
	static void RecordTargetRejectedForAudit(const FCombatSignalTargetPacket& InPacket);
	static void RecordTargetAcceptedForAudit(const FCombatSignalTargetPacket& InPacket);

	static void RecordTimingCueRejectedForAudit(const FCombatSignal& InSignal, const TCHAR* InReason);
	static void RecordTimingCueAcceptedForAudit(const FCombatSignal& InSignal);

public:
	// Target Debug Dump
	static void PrintTargetPacketDebug(const FCombatSignalTargetPacket& InPacket);

public:
	// Shared Dispatch Diagnostic Hook
	static void RecordCombatResultDispatchForAudit(const FCombatSignalTargetPacket& InTargetPacket, const FCombatResultPacket& InResultPacket, const AActor* InReceiverActor, const TCHAR* InEvent);
};
