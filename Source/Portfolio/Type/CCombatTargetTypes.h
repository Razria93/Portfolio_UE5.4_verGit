#pragma once

#include "CoreMinimal.h"
#include "CCombatTargetTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class ECombatTargetChangeReason : uint8
{
	None = 0,

	// Player Policy
	PlayerSelection,

	// AI Participation Lifecycle
	ParticipationAssigned,
	ParticipationRevoked,

	// Policy Invalidation
	PolicyInvalidated,

	// Target Lifetime
	TargetEndPlay,
	OwnerLifecycle,

	// Explicit Command
	ManualClear,

	Max,
};

USTRUCT(BlueprintType)
struct FCombatTargetSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	AActor* TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	int32 Revision = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	ECombatTargetChangeReason LastChangeReason = ECombatTargetChangeReason::None;
};

USTRUCT(BlueprintType)
struct FCombatTargetChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	AActor* PreviousTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CombatTarget")
	FCombatTargetSnapshot CurrentSnapshot;
};
