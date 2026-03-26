#pragma once

#include "CoreMinimal.h"
#include "CHealthStructure.generated.h"

UENUM(BlueprintType)
enum class EDeadState : uint8
{
	Alive,
	Dying,
	Dead,
	Reviving
};
