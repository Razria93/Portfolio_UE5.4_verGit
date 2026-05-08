#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Type/CHealthStructure.h"
#include "CAIStructure.generated.h"

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

USTRUCT(BlueprintType)
struct FTargetData
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
    FTargetData() = default;
    FTargetData(const FTargetData&) = default;
    FTargetData& operator=(const FTargetData&) = default;

public:
    bool IsValidData() const
    {
        return IsValid(TargetActor);
    }
};

USTRUCT(BlueprintType)
struct FAIContext
{
    GENERATED_BODY()

public:
    // Perception Context
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
    bool bShouldEngage = false;

    // Reaction Context
    UPROPERTY(Transient)
    bool bIsActiveReaction = false;

    // Dead Context
    UPROPERTY(VisibleAnywhere)
    EDeadState DeadState = EDeadState::Alive;

public:
    FAIContext() = default;
    FAIContext(const FAIContext&) = default;
    FAIContext& operator=(const FAIContext&) = default;

public:
    bool IsValidContext() const
    {
        return IsValid(TargetActor);
    }
};

USTRUCT(BlueprintType)
struct FPatrolPointData
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
    FPatrolPointData() = default;
    FPatrolPointData(const FPatrolPointData&) = default;
    FPatrolPointData& operator=(const FPatrolPointData&) = default;
};

USTRUCT(BlueprintType)
struct FPatrolContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	bool bUsePatrol = false;

	UPROPERTY(Transient)
	bool bPatrolReverse = false;

	UPROPERTY(Transient)
	class ACPatrolPath* PatrolPath = nullptr;

	UPROPERTY(Transient)
	EPatrolMode PatrolMode = EPatrolMode::None;

	UPROPERTY(Transient)
	int32 CurrentIndex = INDEX_NONE;

	UPROPERTY(Transient)
	int32 NextIndex = INDEX_NONE;

	UPROPERTY(Transient)
	FVector CurrentPatrolLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector NextPatrolLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bNeedNextPoint = false;
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
