#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "Type/CCombatResultTypes.h"

class PORTFOLIO_API FCombatResultDebug
{
public:
	// Gate
	static bool ShouldAuditCombatResult();

public:
	// Combat Result Receiver Diagnostic Hook
	static void RecordCombatResultReceivedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket);

public:
	// Parry Result Diagnostic Hook
	static void RecordParryStackUpdatedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket, int32 InCount, int32 InThreshold, bool bInStaggerReady);
	static void RecordParryStaggerReactionRequestedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const FReactionRequestResult& InResult);
	static void RecordParryStaggerReactionRejectedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket, const TCHAR* InReason);
};
