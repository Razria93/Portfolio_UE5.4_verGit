#pragma once

#include "CoreMinimal.h"
#include "CAIStateStructure.generated.h"

UENUM(BlueprintType)
enum class EAIStateType : uint8
{
	Idle = 0,
	Patrol,
	Investigate,
	Chase,
	Alert,
	Combat,
	HitReact,
	Dead,
	Max,
};