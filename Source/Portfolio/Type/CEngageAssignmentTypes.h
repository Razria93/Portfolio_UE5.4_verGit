#pragma once

#include "CoreMinimal.h"

class ACAIController;
class AActor;

#include "CEngageAssignmentTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
	None,
	Engage,
	Alert
};

UENUM(BlueprintType)
enum class EAIUpdatePrecision : uint8
{
	High,
	Reduced,
	Low
};

USTRUCT(BlueprintType)
struct FEngageRequestContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ACAIController* RequestController = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	int TargetPriority = INT_MAX;

	UPROPERTY(Transient)
	float DistanceToTarget = 0.f;

	UPROPERTY(Transient)
	bool bWasEngaged = false;

public:
	FEngageRequestContext() = default;
};

USTRUCT(BlueprintType)
struct FEngageAssignmentContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	ECombatRole CombatRole = ECombatRole::None;

public:
	FEngageAssignmentContext() = default;

public:
	bool IsValidAssignment() const
	{
		return IsValid(TargetActor) && CombatRole != ECombatRole::None;
	}
};

struct FEngageAssignmentSlotState
{
	int32 EngageCount = 0;
	int32 AlertCount = 0;
};

struct FEngageAssignmentRebuildDebugState
{
	int32 RequestSnapshotCount = 0;
	int32 RequestBucketCount = 0;
	int32 WarmupRequestCount = 0;
	int32 FreshAppliedCount = 0;
	int32 PromotedCount = 0;
	int32 PreservedEngageCount = 0;
	int32 PreservedAlertCount = 0;
};
