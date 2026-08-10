#pragma once

#include "CoreMinimal.h"
#include "CTargetingTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class ETargetSwitchDirection : uint8
{
	Left,
	Right,

	Max,
};

USTRUCT(BlueprintType)
struct FTargetingTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0"))
	float MaxTargetDistance = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxTargetAngleDegrees = 55.f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0"))
	float DistanceScoreWeight = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0"))
	float AngleScoreWeight = 0.65f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.01"))
	float ValidationInterval = 0.1f;
};

struct FTargetingDebugSnapshot
{
	TWeakObjectPtr<AActor> TargetActor;
	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewForward = FVector::ForwardVector;
	FVector TargetLocation = FVector::ZeroVector;
	float Distance = 0.f;
	float MaxTargetDistance = 0.f;
	float Dot = 0.f;
	float MinDot = 0.f;
	float AngleScore = 0.f;
	float DistanceScore = 0.f;
	float FinalScore = 0.f;
	bool bWithinRange = false;
	bool bWithinViewCone = false;
};
