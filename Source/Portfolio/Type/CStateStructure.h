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
	Max,
};
