#pragma once

#include "CoreMinimal.h"
#include "CCharacterFeedbackTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EDeathPresentationReason : uint8
{
	None = 0,
	DeadInCompleted,
	DeadInStartFailed,
	DeadInInterrupted,

	Max
};

UENUM(BlueprintType)
enum class EDeathPresentationRuntimeState : uint8
{
	Inactive = 0,
	Requested,
	Active,

	Max
};

UENUM()
enum class EDeathPresentationEventType : uint8
{
	Started = 0,
	Unavailable,
	Finished,

	Max
};

UENUM(BlueprintType)
enum class EDeathFinalizeReason : uint8
{
	None = 0,
	PresentationCompleted,
	PresentationFallbackExpired,

	Max
};
