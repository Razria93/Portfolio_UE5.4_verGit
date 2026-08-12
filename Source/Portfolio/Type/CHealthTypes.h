#pragma once

#include "CoreMinimal.h"
#include "CHealthTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EDeadState : uint8
{
	Alive = 0,
	Dead = 2,
};

UENUM(BlueprintType)
enum class EMaxHPUpdatePolicy : uint8
{
	ClampCurrent = 0,
	FillToMax
};
