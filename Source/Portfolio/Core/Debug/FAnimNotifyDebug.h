#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "Type/CReactionTypes.h"

class PORTFOLIO_API FAnimNotifyDebug
{
public:
	// Action Notify Warning Report
	static void ReportActionNotifyTriggerWarning(const UObject* InNotifyObject, const AActor* InOwnerActor, const UObject* InComponent, EActionType InTriggerActionType, int32 InTriggerActionIndex, const TCHAR* InReason);

public:
	// Reaction Notify Warning Report
	static void ReportReactionNotifyTriggerWarning(const UObject* InNotifyObject, const AActor* InOwnerActor, const UObject* InComponent, EReactionType InTriggerReactionType, const TCHAR* InReason);
};
