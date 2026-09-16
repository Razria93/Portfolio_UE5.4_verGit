#pragma once

#include "CoreMinimal.h"
#include "CActionTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EActionType : uint8
{
	None = 0,

	Idle,

	Equip,
	Unequip,

	ComboAttack,
	Execution,

	Guard,
	Dodge,

	All,

	Max,
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

	Complete = 1,

	PushHitContext = 2,
	ClearHitContext = 3,

	OpenReserveChainWindow = 4,
	CloseReserveChainWindow = 5,
	ConsumeChain = 6,

	Equip = 7,
	Unequip = 8,

	SwitchToGuard = 9,
	AllowGuardStart = 10,
	CommitExecution = 11,

	EquipSocketTransformTransition = 12,
	UnequipSocketTransformTransition = 13,

	Max = 14,
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
