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
enum class EDeferredActionConsumeKey : uint8
{
	None = 0,

	AfterGuardInAction,
	AfterGuardBlockReaction,

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

public:
	FString ToDebugString() const
	{
		return FString::Printf(
			TEXT("Source=%s | Intent=%s | Event=%s"),
			*UEnum::GetValueAsString(IntentSource),
			*UEnum::GetValueAsString(IntentType),
			*UEnum::GetValueAsString(IntentEvent)
		);
	}
};

USTRUCT(BlueprintType)
struct FActionRequestResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionRequestResultType ResultType = EActionRequestResultType::None;

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

	bool IsAccepted() const
	{
		return ResultType == EActionRequestResultType::Handled
			|| ResultType == EActionRequestResultType::Started
			|| ResultType == EActionRequestResultType::Reserved
			|| ResultType == EActionRequestResultType::Deferred
			|| ResultType == EActionRequestResultType::Intervened;
	}

	bool IsHandledResult() const
	{
		return ResultType == EActionRequestResultType::Handled;
	}

	bool IsStartedResult() const
	{
		return ResultType == EActionRequestResultType::Started;
	}

	bool IsReservedResult() const
	{
		return ResultType == EActionRequestResultType::Reserved;
	}

	bool IsDeferredResult() const
	{
		return ResultType == EActionRequestResultType::Deferred;
	}

	bool IsIntervenedResult() const
	{
		return ResultType == EActionRequestResultType::Intervened;
	}
};

USTRUCT(BlueprintType)
struct FActionCandidate
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionDataKey ActionDataKey = FActionDataKey();

public:
	bool IsValidMinimal() const
	{
		return ActionDataKey.IsValidMinimal();
	}
};

USTRUCT(BlueprintType)
struct FDeferredActionCandidate
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EDeferredActionConsumeKey ConsumeKey = EDeferredActionConsumeKey::None;

	UPROPERTY(Transient)
	FActionCandidate Candidate = FActionCandidate();

public:
	bool IsValidMinimal() const
	{
		return ConsumeKey != EDeferredActionConsumeKey::None
			&& ConsumeKey != EDeferredActionConsumeKey::Max
			&& Candidate.IsValidMinimal();
	}

	bool MatchesIdentity(EDeferredActionConsumeKey InConsumeKey, const FActionCandidate& InCandidate) const
	{
		return ConsumeKey == InConsumeKey
			&& Candidate.ActionDataKey == InCandidate.ActionDataKey;
	}
};
