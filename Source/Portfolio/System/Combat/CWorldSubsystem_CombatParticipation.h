#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/Debug/FCombatEngageDebugTypes.h"
#include "Type/CCombatParticipationTypes.h"
#include "Type/CEngageAssignmentTypes.h"
#include "Type/CHealthTypes.h"
#include "CWorldSubsystem_CombatParticipation.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnCombatParticipationChanged, class ACAIController*, const FCombatParticipationChange&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatParticipationEvidenceExhausted, const FCombatParticipationEvidenceExhaustedEvent&);

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

// CombatParticipationPair means one Participant × Target combination.
// It is a neutral identity key, not a Candidate lifecycle object.
struct FCombatParticipationPairKey
{
	ACAIController* Participant = nullptr;
	AActor* TargetActor = nullptr;

	bool operator==(const FCombatParticipationPairKey& InOther) const
	{
		return Participant == InOther.Participant && TargetActor == InOther.TargetActor;
	}

	friend uint32 GetTypeHash(const FCombatParticipationPairKey& InKey)
	{
		return HashCombine(::GetTypeHash(InKey.Participant), ::GetTypeHash(InKey.TargetActor));
	}
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

private:
	// Evidence Registry
	TMap<FCombatParticipationEvidenceKey, FCombatParticipationEvidence> EvidenceRegistry;
	TMap<FCombatParticipationPairKey, FCombatParticipationLastKnownTargetContext> LastKnownTargetContextByParticipationPair;
	TArray<FCombatParticipationEvidenceExhaustedEvent> PendingEvidenceExhaustedEvents;
	TSet<class ACAIController*> SuppressedParticipants;

private:
	// Target Lifecycle Binding
	TSet<class AActor*> TargetEndPlayBindings;
	TSet<class UCHealthComponent*> TargetHealthBindings;

private:
	// Assignment State
	UPROPERTY()
	TMap<class ACAIController*, FEngageAssignmentContext> AssignmentByParticipant;

private:
	// Assignment Lock State
	TMap<class ACAIController*, FCombatParticipationAssignmentLock> AssignmentLockByParticipant;

public:
	// -----------------------------------------------------------------------------
	// Public API
	// -----------------------------------------------------------------------------
	FOnCombatParticipationChanged OnCombatParticipationChanged;
	FOnCombatParticipationEvidenceExhausted OnCombatParticipationEvidenceExhausted;

	// Lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// Evidence Ingress
	void ReportEvidence(class ACAIController* InParticipant, ECombatParticipationSource InSource, class AActor* InTarget, const FCombatParticipationEvidenceContext& InContext);
	void ReportHitReactiveEvidence(class ACAIController* InParticipant, class AActor* InTarget, const FCombatParticipationEvidenceContext& InContext, uint64 InResultSerial);

	// HitReactive Evidence Lifetime
	void StartHitReactiveEvidencePostReactionTTL(class ACAIController* InParticipant, class AActor* InTarget, uint64 InResultSerial);

	// Evidence Removal
	void WithdrawEvidence(class ACAIController* InParticipant, ECombatParticipationSource InSource, class AActor* InTarget, bool bAllowInvestigateHandoff = true);

	// Participation Release
	void SetParticipationSuppressed(class ACAIController* InParticipant, bool bSuppressed);
	void SoftReleaseParticipationForParticipant(class ACAIController* InParticipant);
	void HardReleaseParticipationForParticipant(class ACAIController* InParticipant);

	// Query
	FEngageAssignmentContext GetAssignment(const class ACAIController* InCAIController) const;
	bool HasActiveEvidenceForParticipationPair(const class ACAIController* InParticipant, const class AActor* InTarget) const;
	FCombatParticipationDebugSnapshot BuildDebugSnapshot() const;

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
	void PruneInactiveEvidence();
	void PruneInactiveAssignmentLocks();

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

	// Assignment Result
	void PublishAssignmentChanges(const TMap<class ACAIController*, FEngageAssignmentContext>& InPreviousAssignments);
	void DispatchPendingEvidenceExhaustedEvents();

private:
	// -----------------------------------------------------------------------------
	// Assignment Rebuild Support
	// -----------------------------------------------------------------------------

	// Assignment Warmup Support
	void StartAssignmentWarmupIfNeeded();

	// Evidence Lifecycle Support
	bool IsCombatParticipationPairValid(const class ACAIController* InParticipant, const class AActor* InTarget) const;
	bool IsHitReactiveEvidenceInactive(const FCombatParticipationEvidence& InEvidence) const;
	bool IsHitReactiveEvidenceAnchorExceeded(const FCombatParticipationEvidence& InEvidence) const;
	void UpdateLastKnownTargetContextFromEvidence(const FCombatParticipationEvidence& InEvidence);
	void ProcessEvidenceExhaustion(const FCombatParticipationEvidence& InEvidence, bool bAllowInvestigateHandoff);
	void ClearLastKnownTargetContextForParticipationPair(const FCombatParticipationPairKey& InKey);
	void ClearLastKnownTargetContextsForParticipant(const class ACAIController* InParticipant);
	void ClearLastKnownTargetContextsForTarget(const class AActor* InTarget);

	// Candidate Build Support
	bool IsCandidatePreferred(const FCombatParticipationCandidate& InCandidate, const FCombatParticipationCandidate& InCurrent) const;

	// Assignment Ladder Support
	const FCombatParticipationCandidate* FindCandidate(const TArray<FCombatParticipationCandidate>& InCandidates, const class ACAIController* InParticipant, const class AActor* InTarget) const;
	bool CanAssignCandidateAtRole(const FCombatParticipationCandidate& InCandidate, ECombatRole InRole) const;
	bool TryBuildEngageAssignment(const FCombatParticipationCandidate& InCandidate, FEngageAssignmentContext& OutAssignment, const TMap<class AActor*, struct FEngageAssignmentSlotState>& InSlotState) const;
	bool TryReserveAssignmentSlot(const FEngageAssignmentContext& InAssignment, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState) const;

	// Assignment Retention Support
	bool CanRetainAssignment(const class ACAIController* InCAIController, const FEngageAssignmentContext& InAssignment) const;
	bool IsAssignmentLockActive(const class ACAIController* InParticipant, const FEngageAssignmentContext& InAssignment) const;

private:
	// -----------------------------------------------------------------------------
	// Target Lifecycle
	// -----------------------------------------------------------------------------

	// Lifecycle Binding
	void BindParticipationTargetLifecycle(class AActor* InTarget);
	void BindParticipationTargetEndPlay(class AActor* InTarget);
	void BindParticipationTargetDeadState(class AActor* InTarget);
	void UnbindUnusedTargetLifecycleBindings();

	// Target State Release
	void ReleaseTargetParticipationState(class AActor* InTarget);

	// Lifecycle Callback
	UFUNCTION()
	void HandleTargetEndPlay(class AActor* InTarget, EEndPlayReason::Type InEndPlayReason);
	void HandleTargetDeadStateChanged(EDeadState InPreviousState, EDeadState InNewState, class UCHealthComponent* InHealthComponent);

private:
	// -----------------------------------------------------------------------------
	// Runtime Cleanup
	// -----------------------------------------------------------------------------
	void ClearCombatParticipationRuntimeState();
};
