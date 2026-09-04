#pragma once

#include "CoreMinimal.h"
#include "Type/CAITypes.h"
#include "CEnemyAIConfigTypes.generated.h"

class ACPatrolPath;

// ===== Distance Band =====

USTRUCT(BlueprintType)
struct FEnemyAIDistanceBand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float OffsetRange = 0.f;

	UPROPERTY(EditAnywhere)
	float EnterBuffer = 0.f;

	UPROPERTY(EditAnywhere)
	float ExitBuffer = 0.f;
};

// ===== Policy Config =====

USTRUCT(BlueprintType)
struct FEnemyPatrolConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bUsePatrol = false;

	UPROPERTY(EditAnywhere)
	TObjectPtr<ACPatrolPath> PatrolPath = nullptr;

	UPROPERTY(EditAnywhere)
	EPatrolMode PatrolMode = EPatrolMode::None;
};

USTRUCT(BlueprintType)
struct FEnemyInvestigateConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bUseInvestigate = false;

	UPROPERTY(EditAnywhere)
	float Duration = 0.f;

	UPROPERTY(EditAnywhere)
	int32 MaxIndex = 0;
};

USTRUCT(BlueprintType)
struct FEnemyChaseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FEnemyAIDistanceBand DistanceBand;
};

USTRUCT(BlueprintType)
struct FEnemyAlertConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bUseAlertStep = false;

	UPROPERTY(EditAnywhere)
	float StepSideDistance = 0.f;

	UPROPERTY(EditAnywhere)
	float StepForwardDistance = 0.f;
};

USTRUCT(BlueprintType)
struct FEnemyEngageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FEnemyAIDistanceBand DistanceBand;

	UPROPERTY(EditAnywhere)
	float CombatActionCooldown = 0.f;
};
