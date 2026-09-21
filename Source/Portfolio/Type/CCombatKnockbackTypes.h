#pragma once

#include "CoreMinimal.h"
#include "CCombatKnockbackTypes.generated.h"

// Data / Config

USTRUCT(BlueprintType)
struct FCombatKnockbackSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", Units = "s"))
	float Duration = 0.f;

public:
	bool IsValid() const
	{
		return FMath::IsFinite(Speed) && Speed > 0.f
			&& FMath::IsFinite(Duration) && Duration > 0.f;
	}
};

// Runtime Context

USTRUCT(BlueprintType)
struct FCombatKnockbackContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FCombatKnockbackSpec Spec;

	UPROPERTY(Transient)
	FVector Direction = FVector::ZeroVector;

#if !UE_BUILD_SHIPPING
	// Diagnostic Association
	TWeakObjectPtr<class AActor> DebugSourceActor;
#endif

public:
	bool IsValid() const
	{
		return Spec.IsValid() && !Direction.ContainsNaN()
			&& FMath::IsNearlyZero(Direction.Z) && Direction.IsNormalized();
	}
};
