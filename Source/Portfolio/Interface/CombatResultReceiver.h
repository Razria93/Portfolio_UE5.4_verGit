#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Type/CCombatResultTypes.h"
#include "CombatResultReceiver.generated.h"

UINTERFACE(MinimalAPI)
class UCombatResultReceiver : public UInterface
{
	GENERATED_BODY()
};

class PORTFOLIO_API ICombatResultReceiver
{
	GENERATED_BODY()

public:
	virtual void ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket) = 0;
};
