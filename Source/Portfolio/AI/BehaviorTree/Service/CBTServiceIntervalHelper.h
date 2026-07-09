#pragma once

#include "CoreMinimal.h"

class UBehaviorTreeComponent;

namespace CBTServiceIntervalHelper
{
	float GetAIContextInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetEngageContextInterval();
}
