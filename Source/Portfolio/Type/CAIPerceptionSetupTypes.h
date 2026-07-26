#pragma once

#include "CoreMinimal.h"
#include "CAIPerceptionSetupTypes.generated.h"

// Data / Config

USTRUCT(BlueprintType)
struct FAIControllerPerceptionSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Sight")
	float SightRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Sight")
	float LoseSightRadius = 600.0f;

	UPROPERTY(EditAnywhere, Category = "Sight")
	float PeripheralVisionAngleDegrees = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Sight")
	float MaxAge = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Memory")
	float TargetMemoryTimeout = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Affiliation")
	bool bDetectEnemies = true;

	UPROPERTY(EditAnywhere, Category = "Affiliation")
	bool bDetectFriendlies = false;

	UPROPERTY(EditAnywhere, Category = "Affiliation")
	bool bDetectNeutrals = false;
};
