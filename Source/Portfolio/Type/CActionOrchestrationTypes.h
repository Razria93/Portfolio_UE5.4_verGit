#pragma once

#include "CoreMinimal.h"
#include "Type/CActionTypes.h"
#include "Type/CActionDataTypes.h"
#include "Type/CExecutionTypes.h"
#include "CActionOrchestrationTypes.generated.h"

// Enum

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

UENUM(BlueprintType)
enum class EActionStopReason : uint8
{
	None = 0,

	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EActionFinishReason : uint8
{
	None = 0,

	Completed,
	Interrupted,
	Ignored,

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
	Reserved,
	Deferred,
	Intervened,

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

	InvalidState,
	InvalidEquipment,
	InvalidCombatAction,
	StaleCombatTarget,

	InvalidQuery,

	ActionCandidateNotFound,
	ActionDataNotFound,
	ActionExecutorNotFound,
	RejectedByExecutor,
	NoExecutableAction,

	InvalidIndependent,
	InvalidSequential,
	InvalidExclusive,

	IncomingCannotIntervene,
	ActiveCannotAcceptIntervention,
	InterventionDispatchFailed,
	RejectedByOverlay,

	ActionExecutionFailed,

	Max,
};

// Candidate

USTRUCT(BlueprintType)
struct FActionCandidate
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FActionDataKey ActionDataKey = FActionDataKey();

	UPROPERTY(Transient)
	uint32 ActionRequestSerial = 0;

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

// Request

USTRUCT(BlueprintType)
struct FMovementActionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionIntentSource IntentSource = EActionIntentSource::None;

	UPROPERTY(Transient)
	EMovementActionIntent IntentType = EMovementActionIntent::None;

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
	UPROPERTY(Transient)
	EActionIntentSource IntentSource = EActionIntentSource::None;

	UPROPERTY(Transient)
	EEquipmentActionIntent IntentType = EEquipmentActionIntent::None;

	UPROPERTY(Transient)
	EActionIntentEvent IntentEvent = EActionIntentEvent::None;
};

USTRUCT(BlueprintType)
struct FCombatActionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionIntentSource IntentSource = EActionIntentSource::None;

	UPROPERTY(Transient)
	uint32 ActionRequestSerial = 0;

	UPROPERTY(Transient)
	ECombatActionIntent IntentType = ECombatActionIntent::None;

	UPROPERTY(Transient)
	EActionIntentEvent IntentEvent = EActionIntentEvent::None;

public:
	FString ToDebugString() const
	{
		return FString::Printf(
			TEXT("Source=%s | Serial=%u | Intent=%s | Event=%s"),
			*UEnum::GetValueAsString(IntentSource),
			ActionRequestSerial,
			*UEnum::GetValueAsString(IntentType),
			*UEnum::GetValueAsString(IntentEvent)
		);
	}
};

// Resolution / Resolve Result

USTRUCT(BlueprintType)
struct FActionCombatSignalCueResolution
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bAccepted = false;

	UPROPERTY(Transient)
	FName CueTag = NAME_None;

public:
	bool IsValidResolution() const
	{
		return bAccepted
			&& !CueTag.IsNone();
	}
};

// Result

USTRUCT(BlueprintType)
struct FActionRequestResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EActionRequestResultType ResultType = EActionRequestResultType::None;

	UPROPERTY(Transient)
	uint32 ActionRequestSerial = 0;

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
struct FActionExecutionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	EExecutionRelationship Relationship = EExecutionRelationship::None;

	UPROPERTY(Transient)
	EExecutionApplyMode ApplyMode = EExecutionApplyMode::None;

	UPROPERTY(Transient)
	FActionExecutionContext ResolvedContext = FActionExecutionContext();

	UPROPERTY(Transient)
	EActionRequestRejectReason RejectReason = EActionRequestRejectReason::None;

	UPROPERTY(Transient)
	FExecutionInterventionDirective InterventionDirective = FExecutionInterventionDirective();

	UPROPERTY(Transient)
	TArray<EObservableOverlayHandling> OverlayHandlings;

public:
	bool IsAcceptedDecision() const
	{
		return Decision == EExecutionDecision::Accept;
	}

	bool RequiresIntervention() const
	{
		return InterventionDirective.IsRequested();
	}
};
