#pragma once

#include "CoreMinimal.h"
#include "CAIStateStructure.generated.h"

UENUM(BlueprintType)
enum class EAIStateType : uint8
{
	Wait = 0,
	Patrol,
	Equip,
	Unequip,
	Approach,
	Action,
	Reaction,
	Max,
};