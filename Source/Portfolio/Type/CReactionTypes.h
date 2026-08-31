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
	BlockHit,
	Parry,
	Stagger,

	// Balance Lifecycle
	CollapseIn,
	CollapseOut,
	CollapseHit,

	// Execution Lifecycle
	ExecutionStandard,
	ExecutionRecovery,
	ExecutionLethal,

	// Death Lifecycle
	Dead,

	All,		// Wildcard

	Max,		// Sentinel
};

UENUM(BlueprintType)
enum class EReactionNotifyCommand : uint8
{
	None = 0,

	Complete,
	ResetBalance,

	Max,
};
