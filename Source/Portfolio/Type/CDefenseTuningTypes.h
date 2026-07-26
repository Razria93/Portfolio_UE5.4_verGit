#pragma once

#include "CoreMinimal.h"
#include "CDefenseTuningTypes.generated.h"

// Data / Config

USTRUCT(BlueprintType)
struct FDefenseGuardTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Guard", meta = (ClampMin = "0.0"))
	float GuardDamageTakenMultiplier = 0.5f;
};
