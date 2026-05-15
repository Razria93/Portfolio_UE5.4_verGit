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
	Cancelled,

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
	Cancelled,

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
	ActionCandidateNotFound,
	ActionDataNotFound,
	ActionExecutorNotFound,
	NoExecutableAction,

	Max,
};

UENUM(BlueprintType)
enum class EActionLocalLevelDecision : uint8
{
	None = 0,

	Reject,
	Ignore,

	Start,
	Chain,
	Enqueue,
	Interrupt,
	Cancel,

	Max,
};

UENUM(BlueprintType)
enum class EActionOrchestrationLevelDecision : uint8
{
	None = 0,

	Reject,
	Ignore,

	Handle,

	Start,
	Chain,
	Enqueue,
	Interrupt,
	Cancel,

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

public:
	UPROPERTY(Transient)
	EActionRequestResultType ResultType = EActionRequestResultType::None;

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

	bool IsAccepted() const
	{
		return ResultType == EActionRequestResultType::Handled
			|| ResultType == EActionRequestResultType::Started
			|| ResultType == EActionRequestResultType::Chained
			|| ResultType == EActionRequestResultType::Enqueued
			|| ResultType == EActionRequestResultType::Interrupted
			|| ResultType == EActionRequestResultType::Cancelled;
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
		return ActionDataKey.IsValidExactKey();
	}
};

USTRUCT(BlueprintType)
struct FActionResolvedContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionDataKey ActionDataKey = FActionDataKey();

	UPROPERTY(Transient)
	FActionData ActionData = FActionData();

	UPROPERTY(Transient)
	class UCAction* ActionExecutor = nullptr;

public:
	bool IsValidMinimal() const;
};

USTRUCT(BlueprintType)
struct FActionResolvedPolicy
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bCanStart = false;

	UPROPERTY(Transient)
	bool bCanChain = false;

	UPROPERTY(Transient)
	bool bCanEnqueue = false;

	UPROPERTY(Transient)
	bool bCanInterrupt = false;

	UPROPERTY(Transient)
	bool bCanCancel = false;
};

USTRUCT(BlueprintType)
struct FActionLocalLevelQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionState ExecutionState = EExecutionState::Idle;

	UPROPERTY(Transient)
	bool bIsActiveAction = false;

	UPROPERTY(Transient)
	FActionResolvedContext IncomingContext = FActionResolvedContext();

	UPROPERTY(Transient)
	FActionResolvedContext ActiveContext = FActionResolvedContext();
};

USTRUCT(BlueprintType)
struct FActionLocalLevelResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionLocalLevelDecision Decision = EActionLocalLevelDecision::Reject;

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

public:
	bool IsAcceptedDecision() const
	{
		return Decision == EActionLocalLevelDecision::Start
			|| Decision == EActionLocalLevelDecision::Chain
			|| Decision == EActionLocalLevelDecision::Enqueue
			|| Decision == EActionLocalLevelDecision::Interrupt
			|| Decision == EActionLocalLevelDecision::Cancel;
	}
};

USTRUCT(BlueprintType)
struct FActionOrchestrationLevelQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionIntentSource IntentSource = EActionIntentSource::None;

	UPROPERTY(Transient)
	FActionResolvedContext IncomingContext = FActionResolvedContext();

	UPROPERTY(Transient)
	FActionResolvedContext ActiveContext = FActionResolvedContext();

	UPROPERTY(Transient)
	FActionLocalLevelResult LocalLevelResult = FActionLocalLevelResult();

	UPROPERTY(Transient)
	FActionResolvedPolicy ResolvedPolicy = FActionResolvedPolicy();
};

USTRUCT(BlueprintType)
struct FActionOrchestrationLevelResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionOrchestrationLevelDecision Decision = EActionOrchestrationLevelDecision::None;

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

	UPROPERTY(Transient)
	FActionResolvedContext ResolvedContext = FActionResolvedContext();

public:
	UPROPERTY(Transient)
	FReactionStopDirective StopDirective = FReactionStopDirective();

public:
	bool IsAcceptedDecision() const
	{
		return Decision == EActionOrchestrationLevelDecision::Handle
			|| Decision == EActionOrchestrationLevelDecision::Start
			|| Decision == EActionOrchestrationLevelDecision::Chain
			|| Decision == EActionOrchestrationLevelDecision::Enqueue
			|| Decision == EActionOrchestrationLevelDecision::Interrupt
			|| Decision == EActionOrchestrationLevelDecision::Cancel;
	}
};
