#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionTypes.h"
#include "CBalanceTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EBalanceLifecycleState : uint8
{
	Accumulating = 0,

	// Collapse
	CollapseInPending,
	CollapseInActive,

	CollapseLoopActive,

	CollapseOutPending,
	CollapseOutActive,

	// Execution
	ExecutionPrimaryActive,
	ExecutionPrimaryCommitted,

	ExecutionDownActive,

	ExecutionRecoveryPending,
	ExecutionRecoveryActive,

	Max,
};

UENUM(BlueprintType)
enum class EBalanceAbortReason : uint8
{
	None = 0,

	CollapseInRejected,
	CollapseInInterrupted,

	CollapseOutRejected,
	CollapseOutInterrupted,

	ExecutionRecoveryRejected,
	ExecutionRecoveryInterrupted,

	ResetNotifyMissing,
	OwnerDeath,
	ExecutionCancelled,

	Max,
};

UENUM(BlueprintType)
enum class EIncapacitatedPresentation : uint8
{
	None = 0,
	Collapse,
	ExecutionDown,

	Max,
};

// Result

USTRUCT(BlueprintType)
struct FBalanceAdvanceResult
{
	GENERATED_BODY()

public:
	// Data
	UPROPERTY(Transient)
	int32 PreviousCount = 0;

	UPROPERTY(Transient)
	int32 CurrentCount = 0;

	UPROPERTY(Transient)
	int32 Threshold = 0;

	UPROPERTY(Transient)
	uint32 BalanceLifecycleSerial = 0;

	UPROPERTY(Transient)
	bool bThresholdCrossed = false;

public:
	// Query
	bool ShouldDispatchCollapseIn() const { return bThresholdCrossed && BalanceLifecycleSerial != 0; }
};

// Lifecycle Packet

USTRUCT(BlueprintType)
struct FBalanceLifecyclePacket
{
	GENERATED_BODY()

public:
	// Data
	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(Transient)
	uint32 BalanceLifecycleSerial = 0;
};
