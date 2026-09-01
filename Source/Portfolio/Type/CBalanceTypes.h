#pragma once

#include "CoreMinimal.h"
#include "Type/CReactionTypes.h"
#include "CBalanceTypes.generated.h"

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

	Max,
};

// Full-body incapacity presentation. This is deliberately independent from the
// Balance lifecycle: the lifecycle owns gameplay while montage notifies choose
// the safe visual hand-off moment.
UENUM(BlueprintType)
enum class EIncapacitatedPresentation : uint8
{
	None = 0,
	Collapse,
	ExecutionDown,

	Max,
};

USTRUCT(BlueprintType)
struct FBalanceLifecyclePacket
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EReactionType ReactionType = EReactionType::None;

	UPROPERTY(Transient)
	uint32 BalanceLifecycleSerial = 0;
};

USTRUCT(BlueprintType)
struct FBalanceAdvanceResult
{
	GENERATED_BODY()

public:
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
	bool ShouldDispatchCollapseIn() const { return bThresholdCrossed && BalanceLifecycleSerial != 0; }
};
