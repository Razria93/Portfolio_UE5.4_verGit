#pragma once

#include "CoreMinimal.h"
#include "CExecutionCollaborationTypes.generated.h"

UENUM(BlueprintType)
enum class EExecutionOutcomePolicy : uint8
{
	None = 0,

	Standard,
	Lethal,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionCollaborationState : uint8
{
	None = 0,

	Reserved,
	Starting,
	Active,
	Committed,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionCollaborationCancelReason : uint8
{
	None = 0,

	// Participant / opportunity validation
	InvalidParticipant,
	TargetChanged,
	BalanceOpportunityInvalidated,

	// Startup rejection
	SourceActionRejected,
	TargetReactionRejected,

	// Pre-commit execution interruption
	SourceActionInterrupted,
	TargetReactionInterrupted,

	// Forced participant lifecycle termination
	ParticipantDeath,
	ParticipantEndPlay,

	Max,
};

USTRUCT(BlueprintType)
struct FExecutionSessionId
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	uint32 Serial = 0;

public:
	bool IsValidMinimal() const
	{
		return IsValid(SourceActor) && Serial != 0;
	}

	bool operator==(const FExecutionSessionId& InOther) const
	{
		return SourceActor == InOther.SourceActor && Serial == InOther.Serial;
	}
};

USTRUCT(BlueprintType)
struct FExecutionOpportunityReservation
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionSessionId SessionId = FExecutionSessionId();

	UPROPERTY(Transient)
	uint32 BalanceLifecycleSerial = 0;

	UPROPERTY(Transient)
	float SuspendedLoopRemainingSeconds = 0.f;

public:
	bool IsValidMinimal() const
	{
		return SessionId.IsValidMinimal() && BalanceLifecycleSerial != 0;
	}

	bool Matches(const FExecutionOpportunityReservation& InOther) const
	{
		return IsValidMinimal()
			&& InOther.IsValidMinimal()
			&& SessionId == InOther.SessionId
			&& BalanceLifecycleSerial == InOther.BalanceLifecycleSerial;
	}
};

USTRUCT(BlueprintType)
struct FExecutionCollaborationContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionSessionId SessionId = FExecutionSessionId();

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	int32 SourceTargetRevision = 0;

	UPROPERTY(Transient)
	FExecutionOpportunityReservation OpportunityReservation = FExecutionOpportunityReservation();

	UPROPERTY(Transient)
	EExecutionOutcomePolicy OutcomePolicy = EExecutionOutcomePolicy::Standard;

public:
	bool IsValidMinimal() const
	{
		return SessionId.IsValidMinimal()
			&& IsValid(TargetActor)
			&& SourceTargetRevision > 0
			&& OpportunityReservation.IsValidMinimal()
			&& OutcomePolicy != EExecutionOutcomePolicy::None
			&& OutcomePolicy != EExecutionOutcomePolicy::Max;
	}
};

USTRUCT(BlueprintType)
struct FExecutionOutcomePacket
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FExecutionCollaborationContext CollaborationContext = FExecutionCollaborationContext();

public:
	bool IsValidMinimal() const
	{
		return CollaborationContext.IsValidMinimal();
	}
};
