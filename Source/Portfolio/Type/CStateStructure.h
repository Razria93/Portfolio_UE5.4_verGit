#pragma once

#include "CoreMinimal.h"
#include "CStateStructure.generated.h"

UENUM(BlueprintType)
enum class EStateType : uint8
{
	Idle = 0,
	Equip,
	Unequip,
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
	Combat,
	HitReact,
	Dead,
	Max,
};
