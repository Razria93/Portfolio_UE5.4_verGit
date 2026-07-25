#pragma once

#include "CoreMinimal.h"
#include "CStateTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EExecutionState : uint8
{
	Idle = 0,
	Action,
	Reaction,
	Dead,

	Max
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
