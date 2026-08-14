#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Type/CHealthTypes.h"
#include "Type/CEngageAssignmentTypes.h"
#include "CAITypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EPatrolMode : uint8
{
    None = 0,
    Random,
    Loop,
    Reverse
};

UENUM(BlueprintType)
enum class EPerceptionBuildResult : uint8
{
    Success,    // Target valid
    NoData,     // Target Invalid
    Error       // Error
};

UENUM(BlueprintType)
enum class EContextBuildResult : uint8
{
    Success,
    NoData,
    Error
};

// Runtime State

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

public:
    FPatrolPointSnapshot() = default;
    FPatrolPointSnapshot(const FPatrolPointSnapshot&) = default;
    FPatrolPointSnapshot& operator=(const FPatrolPointSnapshot&) = default;
};

USTRUCT(BlueprintType)
struct FTargetPerceptionState
{
    GENERATED_BODY()

public:
    UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

    UPROPERTY(Transient)
    int TargetPriority = INT_MAX;

    UPROPERTY(Transient)
    bool bHasLOS = false;

    UPROPERTY(Transient)
    float LastSeenTime = 0.f;

    UPROPERTY(Transient)
	FVector LastKnownLocation = FVector::ZeroVector;

public:
    FTargetPerceptionState() = default;
    FTargetPerceptionState(const FTargetPerceptionState&) = default;
    FTargetPerceptionState& operator=(const FTargetPerceptionState&) = default;

public:
    bool IsValidData() const
    {
		return IsValid(TargetActor);
    }
};

// Runtime Context

USTRUCT(BlueprintType)
struct FAIBlackboardUpdateContext
{
    GENERATED_BODY()

public:
    // Perception Context
    UPROPERTY(Transient)
    class AActor* PerceivedTargetActor = nullptr;

    UPROPERTY(Transient)
    int TargetPriority = INT_MAX;

    UPROPERTY(Transient)
    bool bHasLOS = false;

    UPROPERTY(Transient)
    float LastSeenTime = 0.f;

    UPROPERTY(Transient)
    FVector LastKnownLocation = FVector::ZeroVector;

    // Combat Target projection context
    UPROPERTY(Transient)
    class AActor* CombatTargetActor = nullptr;

    UPROPERTY(Transient)
    int32 CombatTargetRevision = 0;

    // Metric Context (Home)
    UPROPERTY(Transient)
    float DistanceToHome = 0.f;

    UPROPERTY(Transient)
    bool bReturnHome = false;

    // Metric Context (Engage)
    UPROPERTY(Transient)
    float DistanceToTarget = 0.f;

    // Alert Context
    UPROPERTY(Transient)
    bool bInAlertRange = false;

    // Engage Context
    UPROPERTY(Transient)
    ECombatRole CombatRole = ECombatRole::None;

    UPROPERTY(Transient)
    bool bShouldEngage = false;

    // Reaction Context
    UPROPERTY(Transient)
    bool bIsActiveReaction = false;

    // Dead Context
    UPROPERTY(VisibleAnywhere)
    EDeadState DeadState = EDeadState::Alive;

public:
    FAIBlackboardUpdateContext() = default;
    FAIBlackboardUpdateContext(const FAIBlackboardUpdateContext&) = default;
    FAIBlackboardUpdateContext& operator=(const FAIBlackboardUpdateContext&) = default;

public:
    bool IsValidContext() const
    {
        return IsValid(CombatTargetActor);
    }
};

USTRUCT(BlueprintType)
struct FEngageContext
{
    GENERATED_BODY()

public:
    UPROPERTY(Transient)
    class AActor* TargetActor = nullptr;

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
    FEngageContext() = default;
    FEngageContext(const FEngageContext&) = default;
    FEngageContext& operator=(const FEngageContext&) = default;

public:
    bool IsValidContext() const
    {
        return IsValid(TargetActor);
    }
};
