#pragma once

#include "CoreMinimal.h"

class AActor;

#include "CCombatSignalTypes.generated.h"

// Enum

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

// Packet

USTRUCT(BlueprintType)
struct FCombatSignalHeader
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ECombatSignalType SignalType = ECombatSignalType::None;

	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	AActor* InstigatorActor = nullptr;

	UPROPERTY(Transient)
	AActor* SignalCauser = nullptr;

	UPROPERTY(Transient)
	FGuid TraceId = FGuid();

	UPROPERTY(Transient)
	int32 SequenceId = INDEX_NONE;

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
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	UPROPERTY(Transient)
	FName SignalTag = NAME_None;

	UPROPERTY(Transient)
	FName CueTag = NAME_None;

	UPROPERTY(Transient)
	float RequestedDamage = 0.0f;

	UPROPERTY(Transient)
	FVector ImpactLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector ImpactNormal = FVector::ZeroVector;

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

// Reserved Pipeline Scaffold

USTRUCT(BlueprintType)
struct FCombatSignalContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FCombatSignal Signal = FCombatSignal();

	UPROPERTY(Transient)
	AActor* ReceiverActor = nullptr;

	UPROPERTY(Transient)
	bool bSourceActorValid = false;

	UPROPERTY(Transient)
	bool bTargetActorValid = false;

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
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	UPROPERTY(Transient)
	ECombatSignalOutcome Outcome = ECombatSignalOutcome::None;

	UPROPERTY(Transient)
	bool bShouldApply = false;

	UPROPERTY(Transient)
	bool bShouldNotifySource = false;

	UPROPERTY(Transient)
	float FinalDamage = 0.0f;

	UPROPERTY(Transient)
	FName ReactionTag = NAME_None;

	UPROPERTY(Transient)
	FName FeedbackTag = NAME_None;

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
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	UPROPERTY(Transient)
	ECombatSignalOutcome Outcome = ECombatSignalOutcome::None;

	UPROPERTY(Transient)
	bool bApplied = false;

	UPROPERTY(Transient)
	bool bDamageCommitted = false;

	UPROPERTY(Transient)
	float CommittedDamage = 0.0f;

	UPROPERTY(Transient)
	bool bReactionRequested = false;

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
	UPROPERTY(Transient)
	FCombatSignalHeader Header = FCombatSignalHeader();

	UPROPERTY(Transient)
	ECombatSignalResultType ResultType = ECombatSignalResultType::None;

	UPROPERTY(Transient)
	ECombatSignalOutcome Outcome = ECombatSignalOutcome::None;

	UPROPERTY(Transient)
	bool bHandled = false;

	UPROPERTY(Transient)
	bool bSucceeded = false;

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
