#pragma once

#include "CoreMinimal.h"
#include "Type/CAIBehaviorTreeTypes.h"

class UBehaviorTreeComponent;

namespace CBTServiceIntervalHelper
{
	// Public API
	float GetAIContextInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetEngageContextInterval();
}
