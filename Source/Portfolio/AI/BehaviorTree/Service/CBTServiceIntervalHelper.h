#pragma once

#include "CoreMinimal.h"
#include "Type/CAIBehaviorTreeTypes.h"

class UBehaviorTreeComponent;

namespace CBTServiceIntervalHelper
{
	// Public API
	float GetDefaultAIContextInterval();
	float GetDefaultAIIntentStateInterval();
	float GetDefaultEngageContextInterval();
	float GetDefaultInvestigateContextInterval();
	float GetDefaultRandomDeviation();

	float GetAIContextInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetEngageContextInterval();
}
