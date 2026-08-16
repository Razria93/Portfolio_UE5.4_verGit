#pragma once

#include "CoreMinimal.h"

class ACAIController;
class AActor;

#include "CEngageAssignmentTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
	None = 0,
	Engage = 1,
	Alert = 2,
	Observe = 3,
	Max,
};

UENUM(BlueprintType)
enum class EEngageAdmissionKind : uint8
{
	None = 0,
	GeneralBase,
	HitReactiveExtra,
	Max,
};

// Data / Config

USTRUCT(BlueprintType)
struct FEngageAssignmentTuning
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Assignment")
	float RebuildInterval = 0.1f;
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
	int32 AssignmentRevision = 0;

	UPROPERTY(Transient)
	ECombatRole CombatRole = ECombatRole::None;

	UPROPERTY(Transient)
	EEngageAdmissionKind EngageAdmission = EEngageAdmissionKind::None;

public:
	FEngageAssignmentContext() = default;

public:
	bool IsValidAssignment() const
	{
		return IsValid(TargetActor) && CombatRole != ECombatRole::None;
	}
};

USTRUCT(BlueprintType)
struct FCombatParticipationChange
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FEngageAssignmentContext PreviousAssignment;

	UPROPERTY(Transient)
	FEngageAssignmentContext CurrentAssignment;
};

// Runtime State

struct FEngageAssignmentSlotState
{
	int32 EngageCount = 0;
	int32 GeneralBaseEngageCount = 0;
	int32 HitReactiveExtraEngageCount = 0;
	int32 AlertCount = 0;
	int32 ObserveCount = 0;
};
