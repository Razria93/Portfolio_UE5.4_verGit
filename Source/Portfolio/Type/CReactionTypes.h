#pragma once

#include "CoreMinimal.h"
#include "CReactionTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EReactionType : uint8
{
	None = 0,	// Invalid, Unset

	Idle,

	Hit,
	Dead,
	BlockHit,
	Parry,
	Stagger,

	All,		// Wildcard

	Max,		// Sentinel
};

UENUM(BlueprintType)
enum class EReactionNotifyCommand : uint8
{
	None = 0,

	Complete,

	Max,
};
