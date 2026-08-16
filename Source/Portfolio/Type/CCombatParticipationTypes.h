#pragma once

#include "CoreMinimal.h"
#include "Type/CEngageAssignmentTypes.h"

class ACAIController;
class AActor;

#include "CCombatParticipationTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatParticipationSource : uint8
{
	Perception = 0,
	HitReactive,
	Max,
};

USTRUCT(BlueprintType)
struct FCombatParticipationEvidenceContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 TargetPriority = INT_MAX;

	UPROPERTY(Transient)
	float DistanceToTarget = 0.f;
};

USTRUCT(BlueprintType)
struct FCombatParticipationEvidence
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	ACAIController* Participant = nullptr;

	UPROPERTY(Transient)
	ECombatParticipationSource Source = ECombatParticipationSource::Perception;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FCombatParticipationEvidenceContext Context;

	UPROPERTY(Transient)
	int32 Generation = 0;

	UPROPERTY(Transient)
	float UpdatedTimeSeconds = 0.f;
};

USTRUCT(BlueprintType)
struct FCombatParticipationAppliedSnapshot
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	int32 AssignmentRevision = 0;

	UPROPERTY(Transient)
	int32 CombatTargetRevision = 0;

	UPROPERTY(Transient)
	ECombatRole CombatRole = ECombatRole::None;

	UPROPERTY(Transient)
	EEngageAdmissionKind EngageAdmission = EEngageAdmissionKind::None;

	UPROPERTY(Transient)
	bool bIsApplied = false;

public:
	bool IsAssigned() const
	{
		return bIsApplied && IsValid(TargetActor) && CombatRole != ECombatRole::None;
	}
};

USTRUCT(BlueprintType)
struct FCombatParticipationActionLock
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	int32 CombatTargetRevision = 0;

	UPROPERTY(Transient)
	int32 AssignmentRevision = 0;

	UPROPERTY(Transient)
	float ExpireTimeSeconds = 0.f;

public:
	bool Matches(const FEngageAssignmentContext& InAssignment) const
	{
		return InAssignment.IsValidAssignment()
			&& InAssignment.CombatRole == ECombatRole::Engage
			&& TargetActor == InAssignment.TargetActor
			&& AssignmentRevision == InAssignment.AssignmentRevision;
	}
};

struct FCombatParticipationCandidate
{
	ACAIController* Participant = nullptr;
	AActor* TargetActor = nullptr;
	int32 TargetPriority = INT_MAX;
	float DistanceToTarget = 0.f;
	bool bHasHitReactiveEvidence = false;
};
