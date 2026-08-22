#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Type/CHealthTypes.h"
#include "Type/CEngageAssignmentTypes.h"
#include "CAITypes.generated.h"

// ===== Enum =====

UENUM(BlueprintType)
enum class EPatrolMode : uint8
{
	None = 0,
	Random,
	Loop,
	Reverse,
};

UENUM(BlueprintType)
enum class EPerceptionBuildResult : uint8
{
	Success,
	NoData,
	Error,
};

UENUM(BlueprintType)
enum class EContextBuildResult : uint8
{
	Success,
	NoData,
	Error,
};

// ===== Patrol =====

USTRUCT(BlueprintType)
struct FPatrolPointSnapshot
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(Transient)
	float ExtraWaitTime = 0.f;

	UPROPERTY(Transient)
	bool bFaceOnArrive = false;

	UPROPERTY(Transient)
	float FaceYaw = 0.f;

	UPROPERTY(Transient)
	FName PointTag = NAME_None;
};

// ===== Perception =====

USTRUCT(BlueprintType)
struct FPerceptionTargetContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	int32 TargetPriority = INT_MAX;

	UPROPERTY(Transient)
	bool bHasLOS = false;

	UPROPERTY(Transient)
	float LastSeenTime = 0.f;

	UPROPERTY(Transient)
	FVector LastKnownLocation = FVector::ZeroVector;

public:
	bool HasTarget() const
	{
		return IsValid(TargetActor);
	}
};

// ===== Blackboard Update =====

USTRUCT(BlueprintType)
struct FCombatParticipationProjectionContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	AActor* CombatTargetActor = nullptr;

	UPROPERTY(Transient)
	int32 CombatTargetRevision = 0;

	UPROPERTY(Transient)
	ECombatRole CombatRole = ECombatRole::None;

	UPROPERTY(Transient)
	int32 CombatParticipationRevision = 0;

	UPROPERTY(Transient)
	bool bHasCombatParticipationProjection = false;

public:
	bool ShouldEngage() const
	{
		return bHasCombatParticipationProjection && CombatRole == ECombatRole::Engage;
	}
};

USTRUCT(BlueprintType)
struct FHomeMetricContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	float DistanceToHome = 0.f;

	UPROPERTY(Transient)
	bool bReturnHome = false;
};

USTRUCT(BlueprintType)
struct FTargetRangeContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	float DistanceToTarget = 0.f;

	UPROPERTY(Transient)
	bool bInAlertRange = false;
};

USTRUCT(BlueprintType)
struct FReactionContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	bool bIsActiveReaction = false;
};

USTRUCT(BlueprintType)
struct FLifecycleContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	EDeadState DeadState = EDeadState::Alive;
};

USTRUCT(BlueprintType)
struct FAIBlackboardUpdateContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FPerceptionTargetContext Perception;

	UPROPERTY(Transient)
	FCombatParticipationProjectionContext CombatParticipation;

	UPROPERTY(Transient)
	FHomeMetricContext Home;

	UPROPERTY(Transient)
	FTargetRangeContext TargetRange;

	UPROPERTY(Transient)
	FReactionContext Reaction;

	UPROPERTY(Transient)
	FLifecycleContext Lifecycle;
};

// ===== Engage Execution =====

USTRUCT(BlueprintType)
struct FEngageContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	float DistanceToTarget = 0.f;

	UPROPERTY(Transient)
	float EngageOffsetRange = 0.f;

	UPROPERTY(Transient)
	float EngageEnterBuffer = 0.f;

	UPROPERTY(Transient)
	float EngageExitBuffer = 0.f;

	UPROPERTY(Transient)
	float EngageOuterRange = 0.f;

	UPROPERTY(Transient)
	float EngageInnerRange = 0.f;

	UPROPERTY(Transient)
	bool bPrevInEngageRange = false;

	UPROPERTY(Transient)
	bool bInEngageRange = false;

	UPROPERTY(Transient)
	bool bCanCombatAction = false;

	UPROPERTY(Transient)
	float NextCombatActionTime = -1.f;

public:
	bool HasTarget() const
	{
		return IsValid(TargetActor);
	}
};
