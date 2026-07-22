#pragma once

#include "CoreMinimal.h"

class UBehaviorTreeComponent;

enum class EBTServiceIntervalPreset : uint8
{
	Default,
	Reduced,
	Aggressive
};

namespace CBTServiceIntervalHelper
{
	float GetAIContextInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp);
	float GetEngageContextInterval();
}
