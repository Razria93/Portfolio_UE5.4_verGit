#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionTypes.h"
#include "Type/CReactionDataTypes.h"
#include "Type/CCombatSignalTargetTypes.h"
#include "Type/CExecutionTypes.h"
#include "CReactionOrchestrationTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EReactionStopReason : uint8
{
	None = 0,

	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EReactionFinishReason : uint8
{
	None = 0,

	Completed,
	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EReactionRequestResultType : uint8
{
	None = 0,

	Rejected,
	Ignored,

	Started,
	Intervened,

	Max,
};

UENUM(BlueprintType)
enum class EReactionRequestRejectReason : uint8
{
	None = 0,

	InvalidOwner,
	InvalidRequest,
	InvalidComponent,

	InvalidDamageResult,

	Dead,

	ReactionCandidateNotFound,
	ReactionDataNotFound,
	ReactionExecutorNotFound,
	RejectedByExecutor,
	NoExecutableReaction,

	InvalidQuery,
	InvalidIndependent,
	InvalidSequential,
	InvalidExclusive,

	IncomingCannotIntervene,
	ActiveCannotAcceptIntervention,
	InterventionDispatchFailed,
	RejectedByOverlay,
	ReactionExecutionFailed,

	Max,
};

UENUM(BlueprintType)
enum class EReactionExecutionLifecycleEventType : uint8
{
	None = 0,

	Started,
	Completed,
	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EReactionIntentSource : uint8
{
	None = 0,

	CombatSignalTarget,
	BalanceLifecycle,

	Max,
};

// Key / Identifier

USTRUCT(BlueprintType)
struct FReactionCandidate
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FReactionDataKey ReactionDataKey = FReactionDataKey();

	UPROPERTY(Transient)
	uint64 CombatSignalResultSerial = 0;

	uint32 BalanceLifecycleSerial = 0;

public:
	bool IsValidMinimal() const
	{
		return ReactionDataKey.IsValidMinimal();
	}
};

// Request

USTRUCT(BlueprintType)
struct FDamageReactionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionIntentSource IntentSource = EReactionIntentSource::CombatSignalTarget;

	UPROPERTY(Transient)
	FCombatSignalTargetPacket CombatSignalTargetPacket = FCombatSignalTargetPacket();
};

USTRUCT(BlueprintType)
struct FBalanceLifecycleReactionRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionIntentSource IntentSource = EReactionIntentSource::BalanceLifecycle;

	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(Transient)
	uint32 BalanceLifecycleSerial = 0;
};

// Result

USTRUCT(BlueprintType)
struct FReactionRequestResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionRequestResultType ResultType = EReactionRequestResultType::None;

	UPROPERTY(Transient)
	EReactionRequestRejectReason RejectReason = EReactionRequestRejectReason::None;

public:
	bool IsAccepted() const
	{
		return ResultType == EReactionRequestResultType::Started
			|| ResultType == EReactionRequestResultType::Intervened;
	}
};

USTRUCT(BlueprintType)
struct FReactionExecutionLifecycleEvent
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionExecutionLifecycleEventType EventType = EReactionExecutionLifecycleEventType::None;

	UPROPERTY(Transient)
	EReactionFinishReason FinishReason = EReactionFinishReason::None;

	UPROPERTY(Transient)
	FReactionExecutionContext Context = FReactionExecutionContext();
};

USTRUCT(BlueprintType)
struct FReactionExecutionResult
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
	FReactionExecutionContext ResolvedContext = FReactionExecutionContext();

	UPROPERTY(Transient)
	EReactionRequestRejectReason RejectReason = EReactionRequestRejectReason::None;

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
