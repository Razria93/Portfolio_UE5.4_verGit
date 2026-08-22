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

	UPROPERTY(Transient)
	FVector ObservedTargetLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector ObservedTargetVelocity = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasTargetObservation = false;
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
	uint64 HitReactiveResultSerial = 0;

	UPROPERTY(Transient)
	float HitReactiveExpireTimeSeconds = 0.f;

	UPROPERTY(Transient)
	bool bHasStartedHitReactivePostReactionTTL = true;

	UPROPERTY(Transient)
	FVector HitReactiveEvidenceAnchorLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasHitReactiveEvidenceAnchor = false;
};

USTRUCT(BlueprintType)
struct FCombatParticipationLastKnownTargetContext
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector LastObservedVelocity = FVector::ZeroVector;

	UPROPERTY(Transient)
	float LastObservedTimeSeconds = 0.f;

	UPROPERTY(Transient)
	ECombatParticipationSource LastObservedSource = ECombatParticipationSource::Perception;

	UPROPERTY(Transient)
	bool bHasObservation = false;
};

USTRUCT(BlueprintType)
struct FCombatParticipationEvidenceExhaustedEvent
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	ACAIController* Participant = nullptr;

	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	FCombatParticipationLastKnownTargetContext LastKnownTargetContext;

	UPROPERTY(Transient)
	ECombatParticipationSource FinalEvidenceSource = ECombatParticipationSource::Perception;

	UPROPERTY(Transient)
	bool bWasAppliedCombatTarget = false;
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
struct FCombatParticipationAssignmentLock
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
	bool bHasPerceptionEvidence = false;
	bool bHasHitReactiveEvidence = false;
};

// Debug Snapshot

struct FCombatParticipationDebugEntry
{
	AActor* ParticipantActor = nullptr;
	AActor* TargetActor = nullptr;
	ECombatRole CombatRole = ECombatRole::None;
	EEngageAdmissionKind EngageAdmission = EEngageAdmissionKind::None;
	int32 AssignmentRevision = 0;
	bool bHasPerceptionEvidence = false;
	bool bHasPerceptionEvidenceLifetimeState = false;
	bool bHasPerceptionLOS = false;
	float PerceptionMemoryRemainingSeconds = 0.f;
	bool bHasHitReactiveEvidence = false;
	bool bHasStartedHitReactivePostReactionTTL = false;
	float HitReactivePostReactionTTLRemainingSeconds = 0.f;
	bool bHasHitReactiveEvidenceAnchor = false;
	FVector HitReactiveEvidenceAnchorLocation = FVector::ZeroVector;
	float HitReactiveEvidenceAnchorRadius = 0.f;
	bool bHasAssignmentLock = false;
};

struct FCombatParticipationDebugTargetSummary
{
	AActor* TargetActor = nullptr;
	int32 EngageCount = 0;
	int32 GeneralBaseEngageCount = 0;
	int32 HitReactiveExtraEngageCount = 0;
	int32 AlertCount = 0;
	int32 ObserveCount = 0;
	int32 GeneralBaseEngageCap = 0;
	int32 HitReactiveExtraEngageCap = 0;
	int32 TotalEngageCap = 0;
	int32 AlertCap = 0;
	int32 ObserveCap = 0;
};

struct FCombatParticipationDebugSnapshot
{
	bool bHasSnapshot = false;
	TArray<FCombatParticipationDebugEntry> Entries;
	TArray<FCombatParticipationDebugTargetSummary> TargetSummaries;
};
