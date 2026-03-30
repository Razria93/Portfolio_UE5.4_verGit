#pragma once

#include "CoreMinimal.h"
#include "CWorldSubSystemStructure.generated.h"

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
	None,
	Engage,
	Alert
};

USTRUCT(BlueprintType)
struct FEngageRequestContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class ACAIController* RequestController = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

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