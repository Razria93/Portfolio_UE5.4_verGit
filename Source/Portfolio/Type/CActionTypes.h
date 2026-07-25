#pragma once

#include "CoreMinimal.h"
#include "CActionTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EActionType : uint8
{
	None = 0,	// Invalid, Unset

	Idle,

	Equip,
	Unequip,

	ComboAttack,

	Guard,
	Dodge,

	All,		// Wildcard

	Max,		// Sentinel
};

UENUM(BlueprintType)
enum class EGuardActionPhase : uint8
{
	None = 0,

	In,
	Out,
	Hold,
	Hit,
	Parry,

	Max,
};

UENUM(BlueprintType)
enum class EActionNotifyCommand : uint8
{
	None = 0,

	Complete,

	PushHitContext,
	ClearHitContext,

	OpenReserveChainWindow,
	CloseReserveChainWindow,
	ConsumeChain,

	Equip,
	Unequip,

	SwitchToGuard,
	AllowGuardStart,

	Max,
};

UENUM(BlueprintType)
enum class EActionEventType : uint8
{
	None = 0,

	ReserveChainWindowOpened,
	ReserveChainWindowClosed,

	ActionStarted,
	ActionCompleted,

	ActionChained,

	ActionInterrupted,
	ActionIgnored,

	Max,
};
