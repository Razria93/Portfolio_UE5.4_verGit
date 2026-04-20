#pragma once

#include "CoreMinimal.h"
#include "CStateStructure.generated.h"

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
enum class EAIStateType : uint8
{
	Idle = 0,
	Patrol,
	Investigate,
	Chase,
	Alert,
	Engage,
	HitReact,
	Dead,
	Max,
};
