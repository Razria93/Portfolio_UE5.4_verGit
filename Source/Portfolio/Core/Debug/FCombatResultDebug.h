#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatResultTypes.h"

class PORTFOLIO_API FCombatResultDebug
{
public:
	// Gate
	static bool ShouldAuditCombatResult();

public:
	// Combat Result Receiver Diagnostic Hook
	static void RecordCombatResultReceivedForAudit(const AActor* InReceiverActor, const FCombatResultPacket& InPacket);

};
