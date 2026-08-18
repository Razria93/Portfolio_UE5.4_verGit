#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/Debug/FCombatEngageDebugTypes.h"
#include "Type/CCombatParticipationTypes.h"
#include "Type/CEngageAssignmentTypes.h"
#include "Type/CHealthTypes.h"
#include "CWorldSubsystem_CombatParticipation.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCombatParticipationChanged, class ACAIController*, const FCombatParticipationChange&);

struct FCombatParticipationEvidenceKey
{
	ACAIController* Participant = nullptr;
	ECombatParticipationSource Source = ECombatParticipationSource::Perception;
	AActor* TargetActor = nullptr;

	bool operator==(const FCombatParticipationEvidenceKey& InOther) const
	{
		return Participant == InOther.Participant && Source == InOther.Source && TargetActor == InOther.TargetActor;
	}

	friend uint32 GetTypeHash(const FCombatParticipationEvidenceKey& InKey)
	{
		return HashCombine(HashCombine(::GetTypeHash(InKey.Participant), ::GetTypeHash(InKey.Source)), ::GetTypeHash(InKey.TargetActor));
	}
};

struct FCombatParticipationCandidateKey
{
	ACAIController* Participant = nullptr;
	AActor* TargetActor = nullptr;

	bool operator==(const FCombatParticipationCandidateKey& InOther) const
	{
		return Participant == InOther.Participant && TargetActor == InOther.TargetActor;
	}

	friend uint32 GetTypeHash(const FCombatParticipationCandidateKey& InKey)
	{
		return HashCombine(::GetTypeHash(InKey.Participant), ::GetTypeHash(InKey.TargetActor));
	}
};

struct FHitReactiveExtraCommitment
{
	AActor* TargetActor = nullptr;
	float ExpireTimeSeconds = 0.f;
};

	namespace CCombatEngageConstants
{
	constexpr float UnsetAssignmentWarmupStartTime = -1.f;
	constexpr int32 InitialAssignmentRebuildId = 0;
	constexpr int32 FirstAssignmentRebuildId = 1;
}

UCLASS()
class PORTFOLIO_API UCWorldSubsystem_CombatParticipation : public UTickableWorldSubsystem
{
	GENERATED_BODY()

private:
	// Config
	UPROPERTY()
	FEngageAssignmentTuning AssignmentTuning;

private:
	// Runtime State
	float ElapsedTime = 0.f;
	float AssignmentWarmupStartTime = CCombatEngageConstants::UnsetAssignmentWarmupStartTime;
	bool bAssignmentWarmupCompleted = false;
	int32 AssignmentRebuildId = CCombatEngageConstants::InitialAssignmentRebuildId;
	int32 AssignmentRevisionSerial = 0;
	int32 EvidenceGenerationSerial = 0;

private:
	// Evidence Registry
	TMap<FCombatParticipationEvidenceKey, FCombatParticipationEvidence> EvidenceRegistry;

private:
	// Target Lifecycle Binding
	TSet<class AActor*> EvidenceTargetEndPlayBindings;
	TSet<class UCHealthComponent*> EvidenceTargetHealthBindings;

private:
	// Assignment State
	UPROPERTY()
	TMap<class ACAIController*, FEngageAssignmentContext> AssignmentByParticipant;
	TMap<class ACAIController*, FHitReactiveExtraCommitment> ExtraAssignmentByParticipant;

private:
	// Assignment Lock State
	TMap<class ACAIController*, FCombatParticipationAssignmentLock> AssignmentLockByParticipant;

public:
	// -----------------------------------------------------------------------------
	// Public API
	// -----------------------------------------------------------------------------
	FOnCombatParticipationChanged OnCombatParticipationChanged;

	// Lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// Query
	FEngageAssignmentContext GetAssignment(const class ACAIController* InCAIController) const;

	// Evidence Ingress
	void ReportEvidence(class ACAIController* InParticipant, ECombatParticipationSource InSource, class AActor* InTarget, const FCombatParticipationEvidenceContext& InContext);
	void WithdrawEvidence(class ACAIController* InParticipant, ECombatParticipationSource InSource, class AActor* InTarget);
	void UnregisterParticipant(class ACAIController* InParticipant);

	// Assignment Lock
	bool AcquireAssignmentLock(class ACAIController* InParticipant, const FCombatParticipationAssignmentLock& InAssignmentLock);
	void ReleaseAssignmentLock(class ACAIController* InParticipant);

private:
	struct FAssignmentRebuildContext;

	// -----------------------------------------------------------------------------
	// Assignment Rebuild
	// -----------------------------------------------------------------------------

	// Rebuild Entry
	void RebuildAssignments();

	// Rebuild Preprocessing
	void PruneInvalidEvidence();
	void PruneExpiredAssignmentLocks();

	// Assignment Warmup
	bool ShouldDelayAssignmentForWarmup() const;
	float GetAssignmentWarmupElapsedTime() const;

	// Candidate Build
	void BuildParticipationCandidates(TArray<FCombatParticipationCandidate>& OutCandidates) const;
	void SortCandidates(TArray<FCombatParticipationCandidate>& InOutCandidates) const;

	// Assignment Ladder
	void PreserveCommittedRole(ECombatRole InRole, FAssignmentRebuildContext& InOutContext) const;
	void PromoteCommittedRole(ECombatRole InFromRole, ECombatRole InToRole, FAssignmentRebuildContext& InOutContext) const;
	void AssignFreshRole(ECombatRole InRole, FAssignmentRebuildContext& InOutContext) const;

	// Assignment Retention
	void UpdateHitReactiveExtraCommitments(const TMap<class ACAIController*, FEngageAssignmentContext>& InPreviousAssignments, const TMap<class ACAIController*, FEngageAssignmentContext>& InNextAssignments);

	// Assignment Result
	void PublishAssignmentChanges(const TMap<class ACAIController*, FEngageAssignmentContext>& InPreviousAssignments);

private:
	// -----------------------------------------------------------------------------
	// Assignment Rebuild Support
	// -----------------------------------------------------------------------------

	// Assignment Warmup Support
	void StartAssignmentWarmupIfNeeded();

	// Rebuild Preprocessing Support
	bool IsCombatParticipationTargetValid(const class ACAIController* InParticipant, const class AActor* InTarget) const;
	bool IsEvidenceExpired(const FCombatParticipationEvidence& InEvidence) const;

	// Candidate Build Support
	bool IsCandidatePreferred(const FCombatParticipationCandidate& InCandidate, const FCombatParticipationCandidate& InCurrent) const;

	// Assignment Ladder Support
	const FCombatParticipationCandidate* FindCandidate(const TArray<FCombatParticipationCandidate>& InCandidates, const class ACAIController* InParticipant, const class AActor* InTarget) const;
	bool CanAssignCandidateAtRole(const FCombatParticipationCandidate& InCandidate, ECombatRole InRole) const;
	bool TryBuildEngageAssignment(const FCombatParticipationCandidate& InCandidate, FEngageAssignmentContext& OutAssignment, const TMap<class AActor*, struct FEngageAssignmentSlotState>& InSlotState) const;
	bool TryReserveAssignmentSlot(const FEngageAssignmentContext& InAssignment, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState) const;

	// Assignment Retention Support
	bool HasEvidenceForTarget(const class ACAIController* InParticipant, const class AActor* InTarget) const;
	bool HasValidEvidenceForAssignment(const class ACAIController* InCAIController, const FEngageAssignmentContext& InAssignment) const;
	bool IsHitReactiveExtraCommitmentActive(const class ACAIController* InParticipant, const FEngageAssignmentContext& InAssignment) const;
	bool IsAssignmentLockActive(const class ACAIController* InParticipant, const FEngageAssignmentContext& InAssignment) const;

private:
	// -----------------------------------------------------------------------------
	// Target Lifecycle
	// -----------------------------------------------------------------------------

	// Lifecycle Binding
	void BindParticipationTargetLifecycle(class AActor* InTarget);
	void BindParticipationTargetEndPlay(class AActor* InTarget);
	void BindParticipationTargetDeadState(class AActor* InTarget);
	void UnbindUnusedParticipationTargetLifecycle();

	// Target State Release
	void ReleaseTargetParticipationState(class AActor* InTarget);

	// Lifecycle Callback
	UFUNCTION()
	void HandleEvidenceTargetEndPlay(class AActor* InTarget, EEndPlayReason::Type InEndPlayReason);
	void HandleEvidenceTargetDeadStateChanged(EDeadState InPreviousState, EDeadState InNewState, class UCHealthComponent* InHealthComponent);

private:
	// -----------------------------------------------------------------------------
	// Runtime Cleanup
	// -----------------------------------------------------------------------------
	void ClearEngageRuntimeState();
};
