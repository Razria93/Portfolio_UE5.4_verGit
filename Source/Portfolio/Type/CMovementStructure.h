#pragma once

#include "CoreMinimal.h"
#include "CMovementStructure.generated.h"

UENUM(BlueprintType)
enum class EMovementGait : uint8
{
	None = 0,

	Walk,
	Run,
	Sprint,

	Max
};