#pragma once

#include "CoreMinimal.h"
#include "CMovementTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EMovementGait : uint8
{
	None = 0,

	Walk,
	Run,
	Sprint,

	Max
};

UENUM(BlueprintType)
enum class EMovementRotationMode : uint8
{
	None = 0,

	OrientToMovement,
	ControllerDesired,
	FixedFacing,

	Max
};

UENUM(BlueprintType)
enum class ELocomotionPresentationMode : uint8
{
	Forward = 0,
	Directional,

	Max
};
