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
enum class EDeathFinalizeReason : uint8
{
	None = 0,
	PresentationCompleted,
	PresentationStartFailed,
	PresentationTimedOut,

	Max
};

// Runtime Result

USTRUCT(BlueprintType)
struct FDeathPresentationStartResult
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	bool bStarted = false;

	UPROPERTY(BlueprintReadOnly)
	float ExpectedDuration = 0.f;
};
