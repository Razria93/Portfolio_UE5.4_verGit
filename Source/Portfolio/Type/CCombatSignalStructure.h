#pragma once

#include "CoreMinimal.h"
#include "CCombatSignalStructure.generated.h"

UENUM(BlueprintType)
enum class ECombatSignalType : uint8
{
	None = 0,

	HitEvidence,
	TimingCue,
	DirectDamage,
	System,

	Max,
};

UENUM(BlueprintType)
enum class ECombatSignalOutcome : uint8
{
	None = 0,

	Hit,
	Blocked,
	Parried,
	Blink,
	Repulse,
	Staggered,
	Dead,

	Max,
};

UENUM(BlueprintType)
enum class ECombatSignalResultType : uint8
{
	None = 0,

	Handled,
	Ignored,
	Rejected,

	Max,
};

USTRUCT(BlueprintType)
struct FCombatSignalHeader
{
	GENERATED_BODY()

public:
	// Signal category.
	UPROPERTY(Transient)
	ECombatSignalType SignalType = ECombatSignalType::None;

	// Signal source.
	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	// Signal target.
	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	// Intent owner.
	UPROPERTY(Transient)
	AActor* InstigatorActor = nullptr;

	// Intent causer.
	UPROPERTY(Transient)
	AActor* SignalCauser = nullptr;

	// Correlation id.
	UPROPERTY(Transient)
	FGuid TraceId = FGuid();

	// Source-local order.
	UPROPERTY(Transient)
	int32 SequenceId = INDEX_NONE;

	// Debug label.
	UPROPERTY(Transient)
	FName DebugTag = NAME_None;

public:
	FCombatSignalHeader() = default;

public:
	bool IsValidMinimal() const
	{
		return SignalType != ECombatSignalType::None
			&& SignalType != ECombatSignalType::Max;
	}
};

USTRUCT(BlueprintType)
struct FCombatSignal
{
	GENERATED_BODY()

public:
	// Signal metadata.
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	// Signal identity.
	UPROPERTY(Transient)
	FName SignalTag = NAME_None;

	// Optional cue identity.
	UPROPERTY(Transient)
	FName CueTag = NAME_None;

	// Pre-evaluation damage.
	UPROPERTY(Transient)
	float RequestedDamage = 0.0f;

	// Hit or cue point.
	UPROPERTY(Transient)
	FVector ImpactLocation = FVector::ZeroVector;

	// Hit normal.
	UPROPERTY(Transient)
	FVector ImpactNormal = FVector::ZeroVector;

	// Signal direction.
	UPROPERTY(Transient)
	FVector Direction = FVector::ZeroVector;

public:
	FCombatSignal() = default;

public:
	bool IsValidMinimal() const
	{
		return Header.IsValidMinimal()
			&& SignalTag != NAME_None;
	}
};

USTRUCT(BlueprintType)
struct FCombatSignalContext
{
	GENERATED_BODY()

public:
	// Incoming signal.
	UPROPERTY(Transient)
	FCombatSignal Signal = FCombatSignal();

	// Actual receiver.
	UPROPERTY(Transient)
	AActor* ReceiverActor = nullptr;

	// Source validity.
	UPROPERTY(Transient)
	bool bSourceActorValid = false;

	// Target validity.
	UPROPERTY(Transient)
	bool bTargetActorValid = false;

	// Receiver validity.
	UPROPERTY(Transient)
	bool bReceiverActorValid = false;

public:
	FCombatSignalContext() = default;

public:
	bool IsValidMinimal() const
	{
		return Signal.IsValidMinimal();
	}
};

USTRUCT(BlueprintType)
struct FCombatSignalEvaluation
{
	GENERATED_BODY()

public:
	// Signal metadata.
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	// Evaluated outcome.
	UPROPERTY(Transient)
	ECombatSignalOutcome Outcome = ECombatSignalOutcome::None;

	// Apply gate.
	UPROPERTY(Transient)
	bool bShouldApply = false;

	// Source notify gate.
	UPROPERTY(Transient)
	bool bShouldNotifySource = false;

	// Evaluated damage.
	UPROPERTY(Transient)
	float FinalDamage = 0.0f;

	// Reaction identity.
	UPROPERTY(Transient)
	FName ReactionTag = NAME_None;

	// Feedback identity.
	UPROPERTY(Transient)
	FName FeedbackTag = NAME_None;

	// Result identity.
	UPROPERTY(Transient)
	FName ResultTag = NAME_None;

public:
	FCombatSignalEvaluation() = default;

public:
	bool IsValidMinimal() const
	{
		return Header.IsValidMinimal()
			&& Outcome != ECombatSignalOutcome::None
			&& Outcome != ECombatSignalOutcome::Max;
	}
};

USTRUCT(BlueprintType)
struct FCombatSignalApplyResult
{
	GENERATED_BODY()

public:
	// Signal metadata.
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	// Applied outcome.
	UPROPERTY(Transient)
	ECombatSignalOutcome Outcome = ECombatSignalOutcome::None;

	// Apply state.
	UPROPERTY(Transient)
	bool bApplied = false;

	// Damage commit state.
	UPROPERTY(Transient)
	bool bDamageCommitted = false;

	// Committed damage.
	UPROPERTY(Transient)
	float CommittedDamage = 0.0f;

	// Reaction request state.
	UPROPERTY(Transient)
	bool bReactionRequested = false;

	// Feedback request state.
	UPROPERTY(Transient)
	bool bFeedbackRequested = false;

public:
	FCombatSignalApplyResult() = default;

public:
	bool IsValidMinimal() const
	{
		return Header.IsValidMinimal()
			&& Outcome != ECombatSignalOutcome::None
			&& Outcome != ECombatSignalOutcome::Max;
	}
};

USTRUCT(BlueprintType)
struct FCombatSignalResult
{
	GENERATED_BODY()

public:
	// Signal metadata.
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	// Result state.
	UPROPERTY(Transient)
	ECombatSignalResultType ResultType = ECombatSignalResultType::None;

	// Result outcome.
	UPROPERTY(Transient)
	ECombatSignalOutcome Outcome = ECombatSignalOutcome::None;

	// Handled flag.
	UPROPERTY(Transient)
	bool bHandled = false;

	// Success flag.
	UPROPERTY(Transient)
	bool bSucceeded = false;

	// Result identity.
	UPROPERTY(Transient)
	FName ResultTag = NAME_None;

public:
	FCombatSignalResult() = default;

public:
	bool IsValidMinimal() const
	{
		return Header.IsValidMinimal()
			&& ResultType != ECombatSignalResultType::None
			&& ResultType != ECombatSignalResultType::Max;
	}

	bool IsHandled() const
	{
		return ResultType == ECombatSignalResultType::Handled;
	}
};
