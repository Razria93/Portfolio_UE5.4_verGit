#pragma once

#include "CoreMinimal.h"
#include "Type/CWeaponStructure.h"
#include "CActionOrchestrationStructure.generated.h"

UENUM(BlueprintType)
enum class EActionIntentSource : uint8
{
	None = 0,

	PlayerInput,
	AI,

	Max,
};

UENUM(BlueprintType)
enum class EActionIntentEvent : uint8
{
	None = 0,

	Started,
	Updated,
	Completed,
	Canceled,

	Max,
};

UENUM(BlueprintType)
enum class EMovementActionIntent : uint8
{
	None = 0,

	Move,
	
	Walk,
	Run,
	Sprint,

	Jump,
	StopJump,

	Max,
};

UENUM(BlueprintType)
enum class EEquipmentActionIntent : uint8
{
	None = 0,

	Toggle,
	Equip,
	Unequip,

	Max,
};

UENUM(BlueprintType)
enum class ECombatActionIntent : uint8
{
	None = 0,

	ComboAttack,
	Guard,
	Dodge,

	Max,
};

UENUM(BlueprintType)
enum class EActionRequestResultType : uint8
{
	None = 0,

	Rejected,
	Ignored,

	Handled,
	Started,
	Chained,
	Enqueued,
	Interrupted,

	Max,
};

UENUM(BlueprintType)
enum class EActionRequestRejectReason : uint8
{
	None = 0,

	InvalidOwner,
	InvalidRequest,
	InvalidComponent,

	Dead,
	InReaction,
	InvalidState,
	InvalidEquipment,
	InvalidCombatAction,

	AlreadyPlaying,
	NoExecutableAction,

	Max,
};

USTRUCT(BlueprintType)
struct FMovementActionRequest
{
	GENERATED_BODY()

public:
	// Who: request source
	UPROPERTY(Transient)
	EActionIntentSource IntentSource = EActionIntentSource::None;

	// What: requested intent
	UPROPERTY(Transient)
	EMovementActionIntent IntentType = EMovementActionIntent::None;

	// How: intent event
	UPROPERTY(Transient)
	EActionIntentEvent IntentEvent = EActionIntentEvent::None;

	UPROPERTY(Transient)
	FVector2D Axis2D = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct FEquipmentActionRequest
{
	GENERATED_BODY()

public:
	// Who: request source
	UPROPERTY(Transient)
	EActionIntentSource IntentSource = EActionIntentSource::None;

	// What: requested intent
	UPROPERTY(Transient)
	EEquipmentActionIntent IntentType = EEquipmentActionIntent::None;

	// How: intent event
	UPROPERTY(Transient)
	EActionIntentEvent IntentEvent = EActionIntentEvent::None;
};

USTRUCT(BlueprintType)
struct FCombatActionRequest
{
	GENERATED_BODY()

public:
	// Who: request source
	UPROPERTY(Transient)
	EActionIntentSource IntentSource = EActionIntentSource::None;

	// What: requested intent
	UPROPERTY(Transient)
	ECombatActionIntent IntentType = ECombatActionIntent::None;

	// How: intent event
	UPROPERTY(Transient)
	EActionIntentEvent IntentEvent = EActionIntentEvent::None;
};

USTRUCT(BlueprintType)
struct FActionRequestResult
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	EActionRequestResultType ResultType = EActionRequestResultType::None;

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

	UPROPERTY(Transient)
	EActionType ResolvedActionType = EActionType::Max;

	bool IsAccepted() const
	{
		return ResultType == EActionRequestResultType::Handled
			|| ResultType == EActionRequestResultType::Started
			|| ResultType == EActionRequestResultType::Chained
			|| ResultType == EActionRequestResultType::Enqueued
			|| ResultType == EActionRequestResultType::Interrupted;
	}
};
