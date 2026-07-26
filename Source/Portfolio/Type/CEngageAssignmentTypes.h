#pragma once

#include "CoreMinimal.h"

class ACAIController;
class AActor;

#include "CEngageAssignmentTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
	None,
	Engage,
	Alert
};

// Data / Config

USTRUCT(BlueprintType)
struct FEngageAssignmentTuning
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Assignment")
	float RebuildInterval = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Assignment")
	float LeaseDuration = 0.5f;
};

// Request

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

// Runtime Context

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

// Runtime State

struct FEngageAssignmentSlotState
{
	int32 EngageCount = 0;
	int32 AlertCount = 0;
};
