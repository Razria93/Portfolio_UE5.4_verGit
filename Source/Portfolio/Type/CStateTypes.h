#pragma once

#include "CoreMinimal.h"
#include "CStateTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EExecutionState : uint8
{
	Idle = 0,
	Action = 1,
	Reaction = 2,

	Max = 4,
};

UENUM(BlueprintType)
enum class EAIIntentState : uint8
{
	Idle = 0,
	Patrol,
	Observe,
	Investigate,
	Chase,
	Alert,
	Engage,
	HitReact,
	Dead,
	Max,
};
