#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAIStructure.generated.h"

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
