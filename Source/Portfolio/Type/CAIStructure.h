#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAIStructure.generated.h"

UENUM(BlueprintType)
enum class EPatrolMode : uint8
{
    None = 0,
    Random,
    Loop,
    Reverse
};

enum class EPerceptionBuildResult : uint8
{
    Success,    // Target valid
    NoData,     // Target Invalid
    Error       // Error
};

enum class EContextBuildResult : uint8
{
    Success,
    NoData,
    Error
};

USTRUCT()
struct FTargetData
{
    GENERATED_BODY()

public:
    UPROPERTY(Transient)
    AActor* TargetActor = nullptr;

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

    // Metric Context
    UPROPERTY(Transient)
    float DistanceToTarget = 0.f;

    UPROPERTY(Transient)
    bool bInRange = false;

    UPROPERTY(Transient)
    float DistanceToHome = 0.f;

    UPROPERTY(Transient)
    bool bReturnHome = false;

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