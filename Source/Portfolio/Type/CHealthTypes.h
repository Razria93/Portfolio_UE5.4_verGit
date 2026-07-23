#pragma once

#include "CoreMinimal.h"
#include "CHealthTypes.generated.h"

UENUM(BlueprintType)
enum class EDeadState : uint8
{
	Alive,
	Dying,
	Dead,
	Reviving
};

UENUM(BlueprintType)
enum class EMaxHPUpdatePolicy : uint8
{
	ClampCurrent = 0,
	FillToMax
};