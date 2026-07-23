#pragma once

#include "CoreMinimal.h"
#include "Type/CExecutionRuleTypes.h"
#include "Type/CActionDataTypes.h"
#include "Type/CReactionDataTypes.h"
#include "Type/CObservableOverlayTypes.h"
#include "Type/CStateTypes.h"
#include "CExecutionTypes.generated.h"

UENUM(BlueprintType)
enum class EExecutionDecision : uint8
{
	None = 0,

	Reject,
	Ignore,

	Accept,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionRelationship : uint8
{
	None = 0,

	Independent,
	Sequential,
	Exclusive,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionApplyMode : uint8
{
	None = 0,

	Start,
	Reserve,
	Intervene,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionStopReason : uint8
{
	None = 0,

	Interrupted,
	Ignored,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionStopSource : uint8
{
	None = 0,

	ActionOrchestration,
	ReactionOrchestration,

	System,
	External,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionAfterStopAction : uint8
{
	None = 0,

	StopOnly,
	StartIncoming,

	Max,
};

USTRUCT(BlueprintType)
struct FExecutionSnapshot
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionState ExecutionState = EExecutionState::Idle;

public:
	UPROPERTY(Transient)
	bool bIsDead = false;

	UPROPERTY(Transient)
	FObservableOverlaySnapshot ObservableOverlay = FObservableOverlaySnapshot();

public:
	bool IsIdle() const
	{
		return ExecutionState == EExecutionState::Idle;
	}

	bool IsInAction() const
	{
		return ExecutionState == EExecutionState::Action;
	}

	bool IsInReaction() const
	{
		return ExecutionState == EExecutionState::Reaction;
	}

	bool IsDead() const
	{
		return bIsDead || ExecutionState == EExecutionState::Dead;
	}

	bool HasObservableOverlay() const
	{
		return ObservableOverlay.HasObservableOverlay();
	}
};

USTRUCT(BlueprintType)
struct FExecutionParticipant
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bIsValid = false;

	UPROPERTY(Transient)
	EExecutionDomain ParticipantDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	FActionExecutionContext ActionContext = FActionExecutionContext();

	UPROPERTY(Transient)
	FReactionExecutionContext ReactionContext = FReactionExecutionContext();

public:
	bool IsValidMinimal() const;
	bool IsActionParticipant() const;
	bool IsReactionParticipant() const;

	const FActionExecutionContext& GetActionContext() const;
	const FReactionExecutionContext& GetReactionContext() const;

	UObject* GetExecutor() const;
	int32 GetPriority() const;
};

USTRUCT(BlueprintType)
struct FObservableOverlayExecutionDecision
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	TArray<EObservableOverlayHandling> Handlings;

public:
	bool IsAccepted() const
	{
		return Decision == EExecutionDecision::Accept;
	}
};

USTRUCT(BlueprintType)
struct FExecutionDecisionQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionSnapshot Snapshot = FExecutionSnapshot();

	UPROPERTY(Transient)
	FExecutionParticipant IncomingPart = FExecutionParticipant();

	UPROPERTY(Transient)
	FExecutionParticipant ActivePart = FExecutionParticipant();

public:
	bool HasIncomingPart() const
	{
		return IncomingPart.IsValidMinimal();
	}

	bool HasActivePart() const
	{
		return ActivePart.IsValidMinimal();
	}
};

USTRUCT(BlueprintType)
struct FObservableOverlayQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionDecisionQuery DecisionQuery = FExecutionDecisionQuery();

	UPROPERTY(Transient)
	EExecutionApplyMode ApplyMode = EExecutionApplyMode::None;
};

USTRUCT(BlueprintType)
struct FExecutionDecisionResult
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	EExecutionDecision Decision = EExecutionDecision::None;

	UPROPERTY(Transient)
	EExecutionRelationship Relationship = EExecutionRelationship::None;

public:
	bool IsAccepted() const
	{
		return Decision == EExecutionDecision::Accept;
	}
};

USTRUCT(BlueprintType)
struct FExecutionInterventionQuery
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionSnapshot Snapshot = FExecutionSnapshot();

	UPROPERTY(Transient)
	FExecutionParticipant IncomingPart = FExecutionParticipant();

	UPROPERTY(Transient)
	FExecutionParticipant ActivePart = FExecutionParticipant();

	UPROPERTY(Transient)
	EExecutionStopReason StopReason = EExecutionStopReason::None;

public:
	bool IsValidMinimal() const
	{
		return IncomingPart.IsValidMinimal()
			&& ActivePart.IsValidMinimal()
			&& StopReason != EExecutionStopReason::None
			&& StopReason != EExecutionStopReason::Max;
	}

};

USTRUCT(BlueprintType)
struct FExecutionInterventionDirective
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bRequested = false;

	UPROPERTY(Transient)
	EExecutionStopSource StopSource = EExecutionStopSource::None;

	UPROPERTY(Transient)
	EExecutionDomain SourceDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	EExecutionDomain TargetDomain = EExecutionDomain::None;

	UPROPERTY(Transient)
	EExecutionStopReason StopReason = EExecutionStopReason::None;

	UPROPERTY(Transient)
	EExecutionAfterStopAction AfterStopAction = EExecutionAfterStopAction::None;

	UPROPERTY(Transient)
	FExecutionParticipant IncomingPart = FExecutionParticipant();

	UPROPERTY(Transient)
	FExecutionParticipant ActivePart = FExecutionParticipant();

public:
	bool IsRequested() const
	{
		return bRequested;
	}

	bool IsValidRequest() const
	{
		return bRequested
			&& StopSource != EExecutionStopSource::None
			&& StopSource != EExecutionStopSource::Max
			&& SourceDomain != EExecutionDomain::None
			&& SourceDomain != EExecutionDomain::Max
			&& TargetDomain != EExecutionDomain::None
			&& TargetDomain != EExecutionDomain::Max
			&& StopReason != EExecutionStopReason::None
			&& StopReason != EExecutionStopReason::Max
			&& IncomingPart.IsValidMinimal()
			&& ActivePart.IsValidMinimal();
	}
};
