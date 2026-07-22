#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Type/CHealthStructure.h"
#include "Type/CWorldSubSystemStructure.h"
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

struct FPerceptionCandidateAuditState
{
    bool bEnabled = false;

    float RuntimeStartTime = 0.f;
    uint64 RuntimeStartFrame = 0;

    float FirstRawPerceptionTime = -1.f;
    uint64 FirstRawPerceptionFrame = 0;

    float FirstValidTargetTime = -1.f;
    uint64 FirstValidTargetFrame = 0;

    int32 RawPerceptionEventCount = 0;
    int32 MaxTargetDataMapSize = 0;

    TSet<TWeakObjectPtr<class AActor>> RawPerceptionActors;
    TSet<TWeakObjectPtr<class AActor>> ValidTargetProviderActors;
    TSet<TWeakObjectPtr<class AActor>> InvalidTargetProviderActors;

    void Reset()
    {
        bEnabled = false;

        RuntimeStartTime = 0.f;
        RuntimeStartFrame = 0;

        FirstRawPerceptionTime = -1.f;
        FirstRawPerceptionFrame = 0;

        FirstValidTargetTime = -1.f;
        FirstValidTargetFrame = 0;

        RawPerceptionEventCount = 0;
        MaxTargetDataMapSize = 0;

        RawPerceptionActors.Reset();
        ValidTargetProviderActors.Reset();
        InvalidTargetProviderActors.Reset();
    }
};

struct FBlackboardEngageLatencyAuditState
{
    bool bEnabled = false;

    float RuntimeStartTime = 0.f;
    uint64 RuntimeStartFrame = 0;

    float FirstPerceptionContextTime = -1.f;
    uint64 FirstPerceptionContextFrame = 0;

    float FirstBlackboardTargetTime = -1.f;
    uint64 FirstBlackboardTargetFrame = 0;

    float FirstEngageRequestTime = -1.f;
    uint64 FirstEngageRequestFrame = 0;

    float FirstEngageAssignmentTime = -1.f;
    uint64 FirstEngageAssignmentFrame = 0;

    TWeakObjectPtr<class AActor> FirstPerceptionTargetActor;
    TWeakObjectPtr<class AActor> FirstBlackboardTargetActor;
    TWeakObjectPtr<class AActor> FirstEngageRequestTargetActor;
    TWeakObjectPtr<class AActor> FirstEngageAssignmentTargetActor;

    void Reset()
    {
        bEnabled = false;

        RuntimeStartTime = 0.f;
        RuntimeStartFrame = 0;

        FirstPerceptionContextTime = -1.f;
        FirstPerceptionContextFrame = 0;

        FirstBlackboardTargetTime = -1.f;
        FirstBlackboardTargetFrame = 0;

        FirstEngageRequestTime = -1.f;
        FirstEngageRequestFrame = 0;

        FirstEngageAssignmentTime = -1.f;
        FirstEngageAssignmentFrame = 0;

        FirstPerceptionTargetActor.Reset();
        FirstBlackboardTargetActor.Reset();
        FirstEngageRequestTargetActor.Reset();
        FirstEngageAssignmentTargetActor.Reset();
    }
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
