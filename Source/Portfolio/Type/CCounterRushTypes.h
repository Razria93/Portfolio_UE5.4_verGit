#pragma once

#include "CoreMinimal.h"
#include "CCounterRushTypes.generated.h"

USTRUCT(BlueprintType)
struct PORTFOLIO_API FCounterRushTuning
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CounterRush", meta = (ClampMin = "0.01", Units = "s"))
	float Duration = .25f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CounterRush", meta = (ClampMin = "0", Units = "cm"))
	float MaxTravelDistance = 450.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CounterRush", meta = (ClampMin = "0", Units = "cm"))
	float ExtraGap = 30.f;
	bool IsValid() const;
};

/** Fixed, planar intent. Collision remains the responsibility of CharacterMovement. */
struct PORTFOLIO_API FCounterRushPlan
{
	FVector Origin = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	FVector DesiredEnd = FVector::ZeroVector;
	FVector LimitedEnd = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	float PlayerRadius = 0.f;
	float TargetRadius = 0.f;
	float CenterSpacing = 0.f;
	float TravelDistance = 0.f;
	bool bValid = false;
	static FCounterRushPlan Build(const FVector& InOrigin, const FVector& InTarget,
		float InPlayerRadius, float InTargetRadius, const FCounterRushTuning& InTuning);
};
