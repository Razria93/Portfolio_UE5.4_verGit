#pragma once

#include "CoreMinimal.h"
#include "CEnemyTargetSelectionTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EEnemyTargetSelectionDecision : uint8
{
	Rejected,
	Unchanged,
	Committed,
	Cleared,
};

UENUM(BlueprintType)
enum class EEnemyTargetSelectionRejectReason : uint8
{
	None,
	MissingCombatTargetComponent,
	InvalidCandidate,
	OwnerActor,
	NoCurrentTarget,
};

USTRUCT(BlueprintType)
struct FEnemyTargetSelectionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	EEnemyTargetSelectionDecision Decision = EEnemyTargetSelectionDecision::Rejected;

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	AActor* CommittedTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	int32 Revision = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	EEnemyTargetSelectionRejectReason RejectReason = EEnemyTargetSelectionRejectReason::None;
};
