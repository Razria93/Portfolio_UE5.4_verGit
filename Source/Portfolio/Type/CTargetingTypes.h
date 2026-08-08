#pragma once

#include "CoreMinimal.h"
#include "CTargetingTypes.generated.h"

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

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug")
	bool bEnableDebugDraw = false;
};
