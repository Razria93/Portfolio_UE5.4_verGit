#pragma once

#include "CoreMinimal.h"
#include "Type/CCombatTargetTypes.h"
#include "CExecutionCollaborationTypes.generated.h"

// Constants

namespace CExecutionActionIndex
{
	constexpr int32 Standard = 0;
	constexpr int32 Lethal = 1;
}

// Enum

UENUM(BlueprintType)
enum class EExecutionOutcomePolicy : uint8
{
	None = 0,

	Standard,
	Lethal,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionLethalCondition : uint8
{
	Disabled = 0,
	HealthRatio,

	Max,
};

UENUM(BlueprintType)
enum class EExecutionCollaborationState : uint8
{
	None = 0,

	Reserved,
	Active,
	Committed,

	Max,
};

UENUM(BlueprintType)
enum class EExternalCombatInputPolicy : uint8
{
	Normal = 0,

	RejectAll,
	DamageOnly,

	Max,
};

enum class EExecutionAvailabilityBlock : uint8
{
	None,

	SessionActive,
	SourceUnavailable,
	NoTarget,
	Geometry,
	TargetUnavailable,
	NoOpportunity,
	MissingData,
	OutcomeUnavailable,
	ExecutionBlocked
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

// Configuration

USTRUCT(BlueprintType)
struct FExecutionStartGeometrySettings
{
	GENERATED_BODY()

public:
	// Data
	UPROPERTY(EditAnywhere, Category = "Execution|Start Geometry", meta = (ClampMin = 0.0))
	float MaxStartDistance = 300.f;

	UPROPERTY(EditAnywhere, Category = "Execution|Start Geometry", meta = (ClampMin = 0.0, ClampMax = 180.0))
	float MaxSourceFacingAngleDegrees = 15.f;

public:
	// Validation
	bool IsValid() const
	{
		return MaxStartDistance > KINDA_SMALL_NUMBER
			&& MaxSourceFacingAngleDegrees > KINDA_SMALL_NUMBER
			&& MaxSourceFacingAngleDegrees <= 180.f;
	}
};

// Session Identity

USTRUCT(BlueprintType)
struct FExecutionSessionId
{
	GENERATED_BODY()

public:
	// Data
	UPROPERTY(Transient)
	AActor* SourceActor = nullptr;

	UPROPERTY(Transient)
	uint32 Serial = 0;

public:
	// Validation
	bool IsValidMinimal() const
	{
		return IsValid(SourceActor) && Serial != 0;
	}

	// Comparison
	bool operator==(const FExecutionSessionId& InOther) const
	{
		return SourceActor == InOther.SourceActor && Serial == InOther.Serial;
	}
};

// Opportunity Reservation

USTRUCT(BlueprintType)
struct FExecutionOpportunityReservation
{
	GENERATED_BODY()

public:
	// Data
	UPROPERTY(Transient)
	FExecutionSessionId SessionId = FExecutionSessionId();

	UPROPERTY(Transient)
	uint32 BalanceLifecycleSerial = 0;

	UPROPERTY(Transient)
	float SuspendedLoopRemainingSeconds = 0.f;

public:
	// Validation
	bool IsValidMinimal() const
	{
		return SessionId.IsValidMinimal() && BalanceLifecycleSerial != 0;
	}

	// Comparison
	bool Matches(const FExecutionOpportunityReservation& InOther) const
	{
		return IsValidMinimal()
			&& InOther.IsValidMinimal()
			&& SessionId == InOther.SessionId
			&& BalanceLifecycleSerial == InOther.BalanceLifecycleSerial;
	}
};

// Runtime Context

USTRUCT(BlueprintType)
struct FExecutionCollaborationContext
{
	GENERATED_BODY()

public:
	// Data
	UPROPERTY(Transient)
	FExecutionSessionId SessionId = FExecutionSessionId();

	UPROPERTY(Transient)
	FCombatTargetSnapshot TargetSnapshot = FCombatTargetSnapshot();

	UPROPERTY(Transient)
	FExecutionOpportunityReservation OpportunityReservation = FExecutionOpportunityReservation();

	UPROPERTY(Transient)
	EExecutionOutcomePolicy OutcomePolicy = EExecutionOutcomePolicy::Standard;

public:
	// Validation
	bool IsValidMinimal() const
	{
		return SessionId.IsValidMinimal()
			&& IsValid(TargetSnapshot.TargetActor)
			&& TargetSnapshot.Revision > 0
			&& OpportunityReservation.IsValidMinimal()
			&& OutcomePolicy != EExecutionOutcomePolicy::None
			&& OutcomePolicy != EExecutionOutcomePolicy::Max;
	}
};

// Outcome Packet

USTRUCT(BlueprintType)
struct FExecutionOutcomePacket
{
	GENERATED_BODY()

public:
	// Data
	UPROPERTY(Transient)
	FExecutionCollaborationContext CollaborationContext = FExecutionCollaborationContext();

	UPROPERTY(Transient)
	float StandardExecutionDamage = 0.f;

public:
	// Validation
	bool IsValidMinimal() const
	{
		if (!CollaborationContext.IsValidMinimal()) return false;
		return CollaborationContext.OutcomePolicy != EExecutionOutcomePolicy::Standard
			|| StandardExecutionDamage > KINDA_SMALL_NUMBER;
	}
};

// Snapshot

struct FExecutionCollaborationRuntimeSnapshot
{
	bool bHasActiveSession = false;
	bool bIsSourceRole = false;
	EExecutionCollaborationState CollaborationState = EExecutionCollaborationState::None;
	FExecutionCollaborationContext CollaborationContext;
	bool bSourceActionTerminal = false;
	bool bTargetReactionTerminal = false;
};

struct FExecutionStartGeometrySnapshot
{
	bool bHasTarget = false;
	AActor* TargetActor = nullptr;
	float CurrentDistance = 0.f;
	float MaxDistance = 0.f;
	float CurrentFacingAngleDegrees = 0.f;
	float MaxFacingAngleDegrees = 0.f;
	bool bIsWithinDistance = false;
	bool bIsWithinFacingAngle = false;
	bool bIsValid = false;
};
