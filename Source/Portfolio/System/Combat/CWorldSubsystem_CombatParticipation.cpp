#include "System/Combat/CWorldSubsystem_CombatParticipation.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CHealthComponent.h"

#include "ProjectGlobal.h"

#include "Controller/CAIController.h"
#include "Core/Debug/FCombatEngageDebug.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"
#include "Core/Profiling/CCombatFeedbackProfiling.h"
#include "Type/CEngageAssignmentTypes.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "AIController.h"

namespace
{
	TAutoConsoleVariable<float> CVarEngageAssignmentWarmupTime(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime"),
		0.0f,
		TEXT("Delays the first CombatEngage assignment rebuild until request candidates are warmed up. 0: disabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentEngageCap(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap"),
		2,
		TEXT("Controls the GeneralBase Engage cap per target for AI Runtime LOD assignment. Default: 2."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentHitReactiveExtraCap(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentHitReactiveExtraCap"),
		3,
		TEXT("Controls the HitReactiveExtra Engage cap per target. Default: 3."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentTotalCap(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentTotalCap"),
		5,
		TEXT("Controls the total Engage cap per target across GeneralBase and HitReactiveExtra. Default: 5."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentAlertCap(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap"),
		6,
		TEXT("Controls max Alert assignees per target for AI Runtime LOD assignment. Default: 6."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentObserveCap(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentObserveCap"),
		12,
		TEXT("Controls max Observe assignees per target for AI Runtime LOD assignment. Default: 12."),
		ECVF_Default);

	TAutoConsoleVariable<float> CVarCombatParticipationHitReactivePostReactionTTL(
		TEXT("Portfolio.AI.CombatParticipation.HitReactivePostReactionTTL"),
		60.0f,
		TEXT("Seconds a HitReactive Evidence remains valid after its correlated Reaction ends. 0: expire immediately."),
		ECVF_Default);

	TAutoConsoleVariable<float> CVarCombatParticipationAssignmentLockTimeout(
		TEXT("Portfolio.AI.CombatParticipation.AssignmentLockTimeout"),
		3.0f,
		TEXT("Maximum seconds a started AI Combat Action can retain its exact Engage assignment. 0: lock expires immediately."),
		ECVF_Default);

	float GetEngageAssignmentWarmupTime()
	{
		return FMath::Max(0.f, CVarEngageAssignmentWarmupTime.GetValueOnGameThread());
	}

	int32 GetEngageAssignmentEngageCap()
	{
		return FMath::Max(0, CVarEngageAssignmentEngageCap.GetValueOnGameThread());
	}

	int32 GetEngageAssignmentHitReactiveExtraCap()
	{
		return FMath::Max(0, CVarEngageAssignmentHitReactiveExtraCap.GetValueOnGameThread());
	}

	int32 GetEngageAssignmentTotalCap()
	{
		return FMath::Max(0, CVarEngageAssignmentTotalCap.GetValueOnGameThread());
	}

	int32 GetEngageAssignmentAlertCap()
	{
		return FMath::Max(0, CVarEngageAssignmentAlertCap.GetValueOnGameThread());
	}

	int32 GetEngageAssignmentObserveCap()
	{
		return FMath::Max(0, CVarEngageAssignmentObserveCap.GetValueOnGameThread());
	}

	float GetHitReactivePostReactionTTL()
	{
		return FMath::Max(0.f, CVarCombatParticipationHitReactivePostReactionTTL.GetValueOnGameThread());
	}

	float GetCombatParticipationAssignmentLockTimeout()
	{
		return FMath::Max(0.f, CVarCombatParticipationAssignmentLockTimeout.GetValueOnGameThread());
	}
}

struct UCWorldSubsystem_CombatParticipation::FAssignmentRebuildContext
{
	const TArray<FCombatParticipationCandidate>& Candidates;
	TMap<ACAIController*, FEngageAssignmentContext>& NextAssignments;
	TMap<AActor*, FEngageAssignmentSlotState>& SlotState;
	FEngageAssignmentRebuildDebugState& DebugState;
};

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

// Lifecycle

void UCWorldSubsystem_CombatParticipation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCWorldSubsystem_CombatParticipation::Deinitialize()
{
	ClearCombatParticipationRuntimeState();

	Super::Deinitialize();
}

// Tick

void UCWorldSubsystem_CombatParticipation::Tick(float DeltaTime)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_CombatEngage_Tick);
	FCombatCollisionProfilingCounters::FlushToCsv();
	FCombatFeedbackProfiling::FlushToCsv();

	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;
	if (ElapsedTime < AssignmentTuning.RebuildInterval) return;

	ElapsedTime = 0.f;
	RebuildAssignments();
}

TStatId UCWorldSubsystem_CombatParticipation::GetStatId() const
{
	// Unreal uses this stat id to track the tickable subsystem.
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCWorldSubsystem_CombatParticipation, STATGROUP_Tickables);
}

// Query

FEngageAssignmentContext UCWorldSubsystem_CombatParticipation::GetAssignment(const ACAIController* InCAIController) const
{
	if (!IsValid(InCAIController)) return FEngageAssignmentContext();

	const FEngageAssignmentContext* found = AssignmentByParticipant.Find(InCAIController);
	if (!found) return FEngageAssignmentContext();

	return *found;
}

FCombatParticipationDebugSnapshot UCWorldSubsystem_CombatParticipation::BuildDebugSnapshot() const
{
	FCombatParticipationDebugSnapshot snapshot;
	TMap<FCombatParticipationCandidateKey, int32> entryIndexByEvidence;

	for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : AssignmentByParticipant)
	{
		ACAIController* participant = pair.Key;
		const FEngageAssignmentContext& assignment = pair.Value;
		if (!IsValid(participant)) continue;

		const FCombatParticipationCandidateKey key{ participant, assignment.TargetActor };
		int32* entryIndex = entryIndexByEvidence.Find(key);
		if (!entryIndex)
		{
			FCombatParticipationDebugEntry& entry = snapshot.Entries.AddDefaulted_GetRef();
			entry.ParticipantActor = participant->GetPawn();
			entry.TargetActor = assignment.TargetActor;
			entryIndexByEvidence.Add(key, snapshot.Entries.Num() - 1);
			entryIndex = entryIndexByEvidence.Find(key);
		}

		FCombatParticipationDebugEntry& entry = snapshot.Entries[*entryIndex];
		entry.CombatRole = assignment.CombatRole;
		entry.EngageAdmission = assignment.EngageAdmission;
		entry.AssignmentRevision = assignment.AssignmentRevision;
		entry.bHasAssignmentLock = IsAssignmentLockActive(participant, assignment);
	}

	for (const TPair<FCombatParticipationEvidenceKey, FCombatParticipationEvidence>& pair : EvidenceRegistry)
	{
		const FCombatParticipationEvidence& evidence = pair.Value;
		if (!IsValid(evidence.Participant)) continue;

		const FCombatParticipationCandidateKey key{ evidence.Participant, evidence.TargetActor };
		int32* entryIndex = entryIndexByEvidence.Find(key);
		if (!entryIndex)
		{
			FCombatParticipationDebugEntry& entry = snapshot.Entries.AddDefaulted_GetRef();
			entry.ParticipantActor = evidence.Participant->GetPawn();
			entry.TargetActor = evidence.TargetActor;
			entryIndexByEvidence.Add(key, snapshot.Entries.Num() - 1);
			entryIndex = entryIndexByEvidence.Find(key);
		}

		FCombatParticipationDebugEntry& entry = snapshot.Entries[*entryIndex];
		entry.bHasPerceptionEvidence |= evidence.Source == ECombatParticipationSource::Perception;
		entry.bHasHitReactiveEvidence |= evidence.Source == ECombatParticipationSource::HitReactive;
	}

	TMap<AActor*, int32> summaryIndexByTarget;
	for (const FCombatParticipationDebugEntry& entry : snapshot.Entries)
	{
		if (!IsValid(entry.TargetActor)) continue;

		int32* summaryIndex = summaryIndexByTarget.Find(entry.TargetActor);
		if (!summaryIndex)
		{
			FCombatParticipationDebugTargetSummary& summary = snapshot.TargetSummaries.AddDefaulted_GetRef();
			summary.TargetActor = entry.TargetActor;
			summary.GeneralBaseEngageCap = GetEngageAssignmentEngageCap();
			summary.HitReactiveExtraEngageCap = GetEngageAssignmentHitReactiveExtraCap();
			summary.TotalEngageCap = GetEngageAssignmentTotalCap();
			summary.AlertCap = GetEngageAssignmentAlertCap();
			summary.ObserveCap = GetEngageAssignmentObserveCap();
			summaryIndexByTarget.Add(entry.TargetActor, snapshot.TargetSummaries.Num() - 1);
			summaryIndex = summaryIndexByTarget.Find(entry.TargetActor);
		}

		FCombatParticipationDebugTargetSummary& summary = snapshot.TargetSummaries[*summaryIndex];
		switch (entry.CombatRole)
		{
		case ECombatRole::Engage:
			++summary.EngageCount;
			if (entry.EngageAdmission == EEngageAdmissionKind::GeneralBase) ++summary.GeneralBaseEngageCount;
			if (entry.EngageAdmission == EEngageAdmissionKind::HitReactiveExtra) ++summary.HitReactiveExtraEngageCount;
			break;
		case ECombatRole::Alert: ++summary.AlertCount; break;
		case ECombatRole::Observe: ++summary.ObserveCount; break;
		default: break;
		}
	}

	snapshot.bHasSnapshot = !snapshot.Entries.IsEmpty() || !snapshot.TargetSummaries.IsEmpty();
	return snapshot;
}

// Evidence Ingress

void UCWorldSubsystem_CombatParticipation::ReportEvidence(ACAIController* InParticipant, ECombatParticipationSource InSource, AActor* InTarget, const FCombatParticipationEvidenceContext& InContext)
{
	if (!IsValid(InParticipant) || !IsValid(InTarget)) return;
	if (SuppressedParticipants.Contains(InParticipant)) return;

	StartAssignmentWarmupIfNeeded();

	const FCombatParticipationEvidenceKey key{ InParticipant, InSource, InTarget };
	FCombatParticipationEvidence& evidence = EvidenceRegistry.FindOrAdd(key);

	evidence.Participant = InParticipant;
	evidence.Source = InSource;
	evidence.TargetActor = InTarget;
	evidence.Context = InContext;

	if (InSource != ECombatParticipationSource::HitReactive)
	{
		evidence.HitReactiveResultSerial = 0;
		evidence.HitReactiveExpireTimeSeconds = 0.f;
		evidence.bHasStartedHitReactivePostReactionTTL = true;
	}

	BindParticipationTargetLifecycle(InTarget);
}

void UCWorldSubsystem_CombatParticipation::ReportHitReactiveEvidence(ACAIController* InParticipant, AActor* InTarget, const FCombatParticipationEvidenceContext& InContext, const uint64 InResultSerial)
{
	if (!IsValid(InParticipant) || !IsValid(InTarget) || InResultSerial == 0) return;

	ReportEvidence(InParticipant, ECombatParticipationSource::HitReactive, InTarget, InContext);

	FCombatParticipationEvidence* evidence = EvidenceRegistry.Find(FCombatParticipationEvidenceKey{ InParticipant, ECombatParticipationSource::HitReactive, InTarget });
	if (!evidence) return;

	evidence->HitReactiveResultSerial = InResultSerial;
	evidence->HitReactiveExpireTimeSeconds = 0.f;
	evidence->bHasStartedHitReactivePostReactionTTL = false;
}

void UCWorldSubsystem_CombatParticipation::StartHitReactivePostReactionTTL(ACAIController* InParticipant, AActor* InTarget, const uint64 InResultSerial)
{
	if (!IsValid(InParticipant) || !IsValid(InTarget) || InResultSerial == 0) return;

	FCombatParticipationEvidence* evidence = EvidenceRegistry.Find(FCombatParticipationEvidenceKey{ InParticipant, ECombatParticipationSource::HitReactive, InTarget });
	if (!evidence || evidence->HitReactiveResultSerial != InResultSerial) return;

	const UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	evidence->HitReactiveExpireTimeSeconds = world->GetTimeSeconds() + GetHitReactivePostReactionTTL();
	evidence->bHasStartedHitReactivePostReactionTTL = true;

	RebuildAssignments();
}

void UCWorldSubsystem_CombatParticipation::WithdrawEvidence(ACAIController* InParticipant, ECombatParticipationSource InSource, AActor* InTarget)
{
	if (!IsValid(InParticipant) || !IsValid(InTarget)) return;

	EvidenceRegistry.Remove(FCombatParticipationEvidenceKey{ InParticipant, InSource, InTarget });
	UnbindUnusedCombatParticipationTargetLifecycle();
}

void UCWorldSubsystem_CombatParticipation::WithdrawAllEvidenceForParticipant(ACAIController* InParticipant)
{
	if (!IsValid(InParticipant)) return;

	bool bChanged = false;
	for (auto iterator = EvidenceRegistry.CreateIterator(); iterator; ++iterator)
	{
		if (iterator.Key().Participant != InParticipant) continue;
		iterator.RemoveCurrent();
		bChanged = true;
	}
	if (!bChanged) return;
	UnbindUnusedCombatParticipationTargetLifecycle();
	RebuildAssignments();
}

void UCWorldSubsystem_CombatParticipation::UnregisterParticipant(ACAIController* InParticipant)
{
	if (!IsValid(InParticipant)) return;
	bool bRemovedState = false;
	SuppressedParticipants.Remove(InParticipant);

	for (auto iterator = EvidenceRegistry.CreateIterator(); iterator; ++iterator)
	{
		if (iterator.Key().Participant == InParticipant)
		{
			iterator.RemoveCurrent();
			bRemovedState = true;
		}
	}

	bRemovedState |= AssignmentByParticipant.Remove(InParticipant) > 0;
	bRemovedState |= AssignmentLockByParticipant.Remove(InParticipant) > 0;
	UnbindUnusedCombatParticipationTargetLifecycle();
	if (bRemovedState) RebuildAssignments();
}

void UCWorldSubsystem_CombatParticipation::SetParticipationSuppressed(ACAIController* InParticipant, const bool bSuppressed)
{
	if (!IsValid(InParticipant)) return;

	if (!bSuppressed)
	{
		SuppressedParticipants.Remove(InParticipant);
		return;
	}

	SuppressedParticipants.Add(InParticipant);
	WithdrawAllEvidenceForParticipant(InParticipant);
}

// Assignment Lock

bool UCWorldSubsystem_CombatParticipation::AcquireAssignmentLock(ACAIController* InParticipant, const FCombatParticipationAssignmentLock& InAssignmentLock)
{
	if (!IsValid(InParticipant) || !IsValid(InAssignmentLock.TargetActor)) return false;

	const FEngageAssignmentContext* assignment = AssignmentByParticipant.Find(InParticipant);
	if (!assignment || !InAssignmentLock.Matches(*assignment)) return false;
	if (!IsCombatParticipationTargetValid(InParticipant, InAssignmentLock.TargetActor)) return false;

	const UWorld* world = GetWorld();
	if (!IsValid(world)) return false;

	FCombatParticipationAssignmentLock assignmentLock = InAssignmentLock;
	assignmentLock.ExpireTimeSeconds = world->GetTimeSeconds() + GetCombatParticipationAssignmentLockTimeout();
	AssignmentLockByParticipant.Add(InParticipant, assignmentLock);
	BindParticipationTargetLifecycle(assignmentLock.TargetActor);
	return true;
}

void UCWorldSubsystem_CombatParticipation::ReleaseAssignmentLock(ACAIController* InParticipant)
{
	if (!IsValid(InParticipant)) return;
	if (AssignmentLockByParticipant.Remove(InParticipant) == 0) return;

	RebuildAssignments();
}

// -----------------------------------------------------------------------------
// Assignment Rebuild
// -----------------------------------------------------------------------------

// Rebuild Entry

void UCWorldSubsystem_CombatParticipation::RebuildAssignments()
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_CombatEngage_RebuildAssignments);

	++AssignmentRebuildId;
	PruneInvalidEvidence();
	PruneExpiredAssignmentLocks();
	UnbindUnusedCombatParticipationTargetLifecycle();

	// Delay assignment rebuild until warmup has completed.
	if (ShouldDelayAssignmentForWarmup())
	{
		FCombatEngageDebug::RecordEngageAssignmentWarmupDelayedForAudit(AssignmentRebuildId, EvidenceRegistry.Num(), GetAssignmentWarmupElapsedTime(), GetEngageAssignmentWarmupTime());

		return;
	}

	bool bCompletedWarmupThisRebuild = false;

	// Mark warmup completion on the first rebuild after the delay.
	if (!bAssignmentWarmupCompleted)
	{
		if (GetEngageAssignmentWarmupTime() > 0.f && AssignmentWarmupStartTime == CCombatEngageConstants::UnsetAssignmentWarmupStartTime) return;

		bAssignmentWarmupCompleted = true;
		bCompletedWarmupThisRebuild = true;
	}

	TMap<ACAIController*, FEngageAssignmentContext> nextAssignments;
	TMap<AActor*, FEngageAssignmentSlotState> slotState;
	FEngageAssignmentRebuildDebugState rebuildDebugState;

	TArray<FCombatParticipationCandidate> candidates;
	BuildParticipationCandidates(candidates);
	SortCandidates(candidates);
	rebuildDebugState.CandidateCount = candidates.Num();
	FAssignmentRebuildContext rebuildContext{ candidates, nextAssignments, slotState, rebuildDebugState };

	PreserveCommittedRole(ECombatRole::Engage, rebuildContext);
	PromoteCommittedRole(ECombatRole::Alert, ECombatRole::Engage, rebuildContext);
	PromoteCommittedRole(ECombatRole::Observe, ECombatRole::Engage, rebuildContext);
	AssignFreshRole(ECombatRole::Engage, rebuildContext);

	PreserveCommittedRole(ECombatRole::Alert, rebuildContext);
	PromoteCommittedRole(ECombatRole::Observe, ECombatRole::Alert, rebuildContext);
	AssignFreshRole(ECombatRole::Alert, rebuildContext);

	PreserveCommittedRole(ECombatRole::Observe, rebuildContext);
	AssignFreshRole(ECombatRole::Observe, rebuildContext);

	if (FCombatEngageDebug::ShouldAuditEngageAssignment() && (bCompletedWarmupThisRebuild || AssignmentRebuildId == CCombatEngageConstants::FirstAssignmentRebuildId))
	{
		FCombatEngageDebug::RecordEngageAssignmentRebuildSummaryForAudit(AssignmentRebuildId, rebuildDebugState, nextAssignments, GetEngageAssignmentEngageCap(), GetEngageAssignmentHitReactiveExtraCap(), GetEngageAssignmentTotalCap(), GetEngageAssignmentAlertCap(), GetEngageAssignmentObserveCap());
	}

	const TMap<ACAIController*, FEngageAssignmentContext> previousAssignments = AssignmentByParticipant;
	AssignmentByParticipant = MoveTemp(nextAssignments);
	UnbindUnusedCombatParticipationTargetLifecycle();
	PublishAssignmentChanges(previousAssignments);
}

// Rebuild Preprocessing

void UCWorldSubsystem_CombatParticipation::PruneInvalidEvidence()
{
	for (auto iterator = EvidenceRegistry.CreateIterator(); iterator; ++iterator)
	{
		const FCombatParticipationEvidence& evidence = iterator.Value();
		if (!IsCombatParticipationTargetValid(evidence.Participant, evidence.TargetActor) || IsEvidenceExpired(evidence))
		{
			iterator.RemoveCurrent();
		}
	}
}

void UCWorldSubsystem_CombatParticipation::PruneExpiredAssignmentLocks()
{
	for (auto iterator = AssignmentLockByParticipant.CreateIterator(); iterator; ++iterator)
	{
		ACAIController* participant = iterator.Key();
		const FCombatParticipationAssignmentLock& assignmentLock = iterator.Value();
		const FEngageAssignmentContext* assignment = AssignmentByParticipant.Find(participant);
		if (!IsValid(participant)
			|| !assignmentLock.Matches(assignment ? *assignment : FEngageAssignmentContext())
			|| !IsCombatParticipationTargetValid(participant, assignmentLock.TargetActor))
		{
			iterator.RemoveCurrent();
			continue;
		}

		const UWorld* world = GetWorld();
		if (!IsValid(world) || world->GetTimeSeconds() >= assignmentLock.ExpireTimeSeconds)
		{
			iterator.RemoveCurrent();
		}
	}
}

// Assignment Warmup

bool UCWorldSubsystem_CombatParticipation::ShouldDelayAssignmentForWarmup() const
{
	if (bAssignmentWarmupCompleted) return false;
	if (AssignmentWarmupStartTime == CCombatEngageConstants::UnsetAssignmentWarmupStartTime) return false;
	if (GetEngageAssignmentWarmupTime() <= 0.f) return false;

	return GetAssignmentWarmupElapsedTime() < GetEngageAssignmentWarmupTime();
}

float UCWorldSubsystem_CombatParticipation::GetAssignmentWarmupElapsedTime() const
{
	if (AssignmentWarmupStartTime == CCombatEngageConstants::UnsetAssignmentWarmupStartTime) return 0.f;

	const UWorld* world = GetWorld();
	if (!IsValid(world)) return 0.f;

	return FMath::Max(0.f, world->GetTimeSeconds() - AssignmentWarmupStartTime);
}

// Candidate Build

void UCWorldSubsystem_CombatParticipation::BuildParticipationCandidates(TArray<FCombatParticipationCandidate>& OutCandidates) const
{
	TMap<FCombatParticipationCandidateKey, FCombatParticipationCandidate> candidatesByKey;

	for (const TPair<FCombatParticipationEvidenceKey, FCombatParticipationEvidence>& pair : EvidenceRegistry)
	{
		const FCombatParticipationEvidence& evidence = pair.Value;
		if (!IsCombatParticipationTargetValid(evidence.Participant, evidence.TargetActor) || IsEvidenceExpired(evidence)) continue;

		FCombatParticipationCandidate& candidate = candidatesByKey.FindOrAdd(FCombatParticipationCandidateKey{ evidence.Participant, evidence.TargetActor });
		candidate.Participant = evidence.Participant;
		candidate.TargetActor = evidence.TargetActor;
		candidate.TargetPriority = FMath::Min(candidate.TargetPriority, evidence.Context.TargetPriority);

		const APawn* participantPawn = evidence.Participant->GetPawn();
		candidate.DistanceToTarget = IsValid(participantPawn) ? FVector::Dist(participantPawn->GetActorLocation(), evidence.TargetActor->GetActorLocation()) : evidence.Context.DistanceToTarget;

		candidate.bHasPerceptionEvidence |= evidence.Source == ECombatParticipationSource::Perception;
		candidate.bHasHitReactiveEvidence |= evidence.Source == ECombatParticipationSource::HitReactive;
	}

	candidatesByKey.GenerateValueArray(OutCandidates);
}

void UCWorldSubsystem_CombatParticipation::SortCandidates(TArray<FCombatParticipationCandidate>& InOutCandidates) const
{
	InOutCandidates.Sort([](const FCombatParticipationCandidate& A, const FCombatParticipationCandidate& B)
		{
			if (A.TargetPriority != B.TargetPriority) return A.TargetPriority < B.TargetPriority;
			if (!FMath::IsNearlyEqual(A.DistanceToTarget, B.DistanceToTarget)) return A.DistanceToTarget < B.DistanceToTarget;
			if (A.TargetActor != B.TargetActor) return GetTypeHash(A.TargetActor) < GetTypeHash(B.TargetActor);

			return GetTypeHash(A.Participant) < GetTypeHash(B.Participant);
		});
}

// Assignment Ladder

void UCWorldSubsystem_CombatParticipation::PreserveCommittedRole(const ECombatRole InRole, FAssignmentRebuildContext& InOutContext) const
{
	const TArray<FCombatParticipationCandidate>& InCandidates = InOutContext.Candidates;
	TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments = InOutContext.NextAssignments;
	TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState = InOutContext.SlotState;
	FEngageAssignmentRebuildDebugState& InOutDebugState = InOutContext.DebugState;

	const EEngageAdmissionKind admissionOrder[]
	{
		EEngageAdmissionKind::GeneralBase,
		EEngageAdmissionKind::HitReactiveExtra,
		EEngageAdmissionKind::None,
	};
	const int32 admissionPassCount = InRole == ECombatRole::Engage ? UE_ARRAY_COUNT(admissionOrder) : 1;

	for (int32 admissionPass = 0; admissionPass < admissionPassCount; ++admissionPass)
	{
		for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : AssignmentByParticipant)
		{
			ACAIController* participant = pair.Key;
			const FEngageAssignmentContext& previousAssignment = pair.Value;
			if (!IsValid(participant) || InOutNextAssignments.Contains(participant)) continue;
			if (!previousAssignment.IsValidAssignment() || previousAssignment.CombatRole != InRole) continue;
			if (InRole == ECombatRole::Engage && previousAssignment.EngageAdmission != admissionOrder[admissionPass]) continue;

			const FCombatParticipationCandidate* candidate = FindCandidate(InCandidates, participant, previousAssignment.TargetActor);
			const bool bHasActiveAssignmentLock = InRole == ECombatRole::Engage && IsAssignmentLockActive(participant, previousAssignment);
			if (!candidate && !bHasActiveAssignmentLock) continue;

			FEngageAssignmentContext assignment = previousAssignment;
			bool bReserved = false;
			if (InRole == ECombatRole::Engage)
			{
				if (bHasActiveAssignmentLock)
				{
					bReserved = TryReserveAssignmentSlot(assignment, InOutSlotState);
				}
				if (!bReserved && candidate && TryBuildEngageAssignment(*candidate, assignment, InOutSlotState))
				{
					bReserved = TryReserveAssignmentSlot(assignment, InOutSlotState);
				}
			}
			else
			{
				bReserved = TryReserveAssignmentSlot(assignment, InOutSlotState);
			}

			if (!bReserved) continue;

			InOutNextAssignments.Add(participant, assignment);
			switch (InRole)
			{
			case ECombatRole::Engage: ++InOutDebugState.PreservedEngageCount; break;
			case ECombatRole::Alert: ++InOutDebugState.PreservedAlertCount; break;
			case ECombatRole::Observe: ++InOutDebugState.PreservedObserveCount; break;
			default: break;
			}
		}
	}
}

void UCWorldSubsystem_CombatParticipation::PromoteCommittedRole(const ECombatRole InFromRole, const ECombatRole InToRole, FAssignmentRebuildContext& InOutContext) const
{
	const TArray<FCombatParticipationCandidate>& InCandidates = InOutContext.Candidates;
	TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments = InOutContext.NextAssignments;
	TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState = InOutContext.SlotState;
	FEngageAssignmentRebuildDebugState& InOutDebugState = InOutContext.DebugState;

	for (const FCombatParticipationCandidate& candidate : InCandidates)
	{
		if (!IsValid(candidate.Participant) || !IsValid(candidate.TargetActor)) continue;
		if (InOutNextAssignments.Contains(candidate.Participant)) continue;

		const FEngageAssignmentContext* previousAssignment = AssignmentByParticipant.Find(candidate.Participant);
		if (!previousAssignment || !previousAssignment->IsValidAssignment()) continue;
		if (previousAssignment->CombatRole != InFromRole || previousAssignment->TargetActor != candidate.TargetActor) continue;

		FEngageAssignmentContext promotedAssignment;
		if (InToRole == ECombatRole::Engage)
		{
			if (!TryBuildEngageAssignment(candidate, promotedAssignment, InOutSlotState)) continue;
		}
		else
		{
			promotedAssignment.TargetActor = candidate.TargetActor;
			promotedAssignment.CombatRole = InToRole;
		}

		if (!TryReserveAssignmentSlot(promotedAssignment, InOutSlotState)) continue;

		InOutNextAssignments.Add(candidate.Participant, promotedAssignment);
		if (InFromRole == ECombatRole::Alert && InToRole == ECombatRole::Engage) ++InOutDebugState.PromotedAlertToEngageCount;
		if (InFromRole == ECombatRole::Observe && InToRole == ECombatRole::Engage) ++InOutDebugState.PromotedObserveToEngageCount;
		if (InFromRole == ECombatRole::Observe && InToRole == ECombatRole::Alert) ++InOutDebugState.PromotedObserveToAlertCount;
	}
}

void UCWorldSubsystem_CombatParticipation::AssignFreshRole(const ECombatRole InRole, FAssignmentRebuildContext& InOutContext) const
{
	const TArray<FCombatParticipationCandidate>& InCandidates = InOutContext.Candidates;
	TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments = InOutContext.NextAssignments;
	TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState = InOutContext.SlotState;
	FEngageAssignmentRebuildDebugState& InOutDebugState = InOutContext.DebugState;

	for (const FCombatParticipationCandidate& candidate : InCandidates)
	{
		if (!IsValid(candidate.Participant) || !IsValid(candidate.TargetActor)) continue;
		if (InOutNextAssignments.Contains(candidate.Participant) || !CanAssignCandidateAtRole(candidate, InRole)) continue;

		FEngageAssignmentContext assignment;
		if (InRole == ECombatRole::Engage)
		{
			if (!TryBuildEngageAssignment(candidate, assignment, InOutSlotState)) continue;
		}
		else
		{
			assignment.TargetActor = candidate.TargetActor;
			assignment.CombatRole = InRole;
		}

		if (!TryReserveAssignmentSlot(assignment, InOutSlotState)) continue;

		InOutNextAssignments.Add(candidate.Participant, assignment);
		switch (InRole)
		{
		case ECombatRole::Engage: ++InOutDebugState.FreshEngageCount; break;
		case ECombatRole::Alert: ++InOutDebugState.FreshAlertCount; break;
		case ECombatRole::Observe: ++InOutDebugState.FreshObserveCount; break;
		default: break;
		}
	}
}

// Assignment Result

void UCWorldSubsystem_CombatParticipation::PublishAssignmentChanges(const TMap<ACAIController*, FEngageAssignmentContext>& InPreviousAssignments)
{
	TSet<ACAIController*> controllers;
	for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : InPreviousAssignments) controllers.Add(pair.Key);
	for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : AssignmentByParticipant) controllers.Add(pair.Key);

	for (ACAIController* controller : controllers)
	{
		const FEngageAssignmentContext* previous = InPreviousAssignments.Find(controller);
		FEngageAssignmentContext* current = AssignmentByParticipant.Find(controller);
		const FEngageAssignmentContext previousAssignment = previous ? *previous : FEngageAssignmentContext();
		FEngageAssignmentContext currentAssignment = current ? *current : FEngageAssignmentContext();
		if (previousAssignment.TargetActor == currentAssignment.TargetActor
			&& previousAssignment.CombatRole == currentAssignment.CombatRole
			&& previousAssignment.EngageAdmission == currentAssignment.EngageAdmission)
		{
			continue;
		}

		currentAssignment.AssignmentRevision = ++AssignmentRevisionSerial;
		if (current) current->AssignmentRevision = currentAssignment.AssignmentRevision;

		FCombatParticipationChange change;
		change.PreviousAssignment = previousAssignment;
		change.CurrentAssignment = currentAssignment;
		OnCombatParticipationChanged.Broadcast(controller, change);
	}
}

// -----------------------------------------------------------------------------
// Assignment Rebuild Support
// -----------------------------------------------------------------------------

// Assignment Warmup Support

void UCWorldSubsystem_CombatParticipation::StartAssignmentWarmupIfNeeded()
{
	if (bAssignmentWarmupCompleted) return;
	if (AssignmentWarmupStartTime != CCombatEngageConstants::UnsetAssignmentWarmupStartTime) return;
	if (GetEngageAssignmentWarmupTime() <= 0.f) return;

	const UWorld* world = GetWorld();
	AssignmentWarmupStartTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
}

// Rebuild Preprocessing Support

bool UCWorldSubsystem_CombatParticipation::IsCombatParticipationTargetValid(const ACAIController* InParticipant, const AActor* InTarget) const
{
	if (!IsValid(InParticipant) || !IsValid(InTarget)) return false;
	if (InParticipant->GetTeamAttitudeTowards(*InTarget) != ETeamAttitude::Hostile) return false;

	const UCHealthComponent* healthComp = InTarget->FindComponentByClass<UCHealthComponent>();
	return !IsValid(healthComp) || healthComp->IsAlive();
}

bool UCWorldSubsystem_CombatParticipation::IsEvidenceExpired(const FCombatParticipationEvidence& InEvidence) const
{
	if (InEvidence.Source != ECombatParticipationSource::HitReactive) return false;
	if (!InEvidence.bHasStartedHitReactivePostReactionTTL) return false;

	const UWorld* world = GetWorld();
	if (!IsValid(world)) return false;

	return world->GetTimeSeconds() >= InEvidence.HitReactiveExpireTimeSeconds;
}

bool UCWorldSubsystem_CombatParticipation::HasActiveEvidenceForParticipantTarget(const ACAIController* InParticipant, const AActor* InTarget) const
{
	if (!IsCombatParticipationTargetValid(InParticipant, InTarget)) return false;

	for (const TPair<FCombatParticipationEvidenceKey, FCombatParticipationEvidence>& pair : EvidenceRegistry)
	{
		const FCombatParticipationEvidence& evidence = pair.Value;
		if (evidence.Participant == InParticipant && evidence.TargetActor == InTarget && !IsEvidenceExpired(evidence)) return true;
	}

	return false;
}

// Candidate Build Support

bool UCWorldSubsystem_CombatParticipation::IsCandidatePreferred(const FCombatParticipationCandidate& InCandidate, const FCombatParticipationCandidate& InCurrent) const
{
	if (InCandidate.TargetPriority != InCurrent.TargetPriority) return InCandidate.TargetPriority < InCurrent.TargetPriority;
	if (!FMath::IsNearlyEqual(InCandidate.DistanceToTarget, InCurrent.DistanceToTarget)) return InCandidate.DistanceToTarget < InCurrent.DistanceToTarget;

	return GetTypeHash(InCandidate.TargetActor) < GetTypeHash(InCurrent.TargetActor);
}

// Assignment Ladder Support

const FCombatParticipationCandidate* UCWorldSubsystem_CombatParticipation::FindCandidate(const TArray<FCombatParticipationCandidate>& InCandidates, const ACAIController* InParticipant, const AActor* InTarget) const
{
	for (const FCombatParticipationCandidate& candidate : InCandidates)
	{
		if (candidate.Participant == InParticipant && candidate.TargetActor == InTarget) return &candidate;
	}

	return nullptr;
}

bool UCWorldSubsystem_CombatParticipation::CanAssignCandidateAtRole(const FCombatParticipationCandidate& InCandidate, const ECombatRole InRole) const
{
	const FEngageAssignmentContext* previousAssignment = AssignmentByParticipant.Find(InCandidate.Participant);
	if (!previousAssignment || !previousAssignment->IsValidAssignment()) return true;
	if (!CanRetainAssignment(InCandidate.Participant, *previousAssignment)) return true;
	if (previousAssignment->TargetActor != InCandidate.TargetActor) return false;
	return static_cast<uint8>(previousAssignment->CombatRole) < static_cast<uint8>(InRole);
}

bool UCWorldSubsystem_CombatParticipation::TryBuildEngageAssignment(const FCombatParticipationCandidate& InCandidate, FEngageAssignmentContext& OutAssignment, const TMap<AActor*, FEngageAssignmentSlotState>& InSlotState) const
{
	if (!IsValid(InCandidate.TargetActor)) return false;

	const FEngageAssignmentSlotState* slotState = InSlotState.Find(InCandidate.TargetActor);
	const FEngageAssignmentSlotState emptySlotState;
	const FEngageAssignmentSlotState& targetSlotState = slotState ? *slotState : emptySlotState;
	if (targetSlotState.EngageCount >= GetEngageAssignmentTotalCap()) return false;

	OutAssignment.TargetActor = InCandidate.TargetActor;
	OutAssignment.CombatRole = ECombatRole::Engage;
	if (targetSlotState.GeneralBaseEngageCount < GetEngageAssignmentEngageCap())
	{
		OutAssignment.EngageAdmission = EEngageAdmissionKind::GeneralBase;
		return true;
	}

	if (InCandidate.bHasHitReactiveEvidence && targetSlotState.HitReactiveExtraEngageCount < GetEngageAssignmentHitReactiveExtraCap())
	{
		OutAssignment.EngageAdmission = EEngageAdmissionKind::HitReactiveExtra;
		return true;
	}

	return false;
}

bool UCWorldSubsystem_CombatParticipation::TryReserveAssignmentSlot(const FEngageAssignmentContext& InAssignment, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState) const
{
	if (!InAssignment.IsValidAssignment()) return false;

	FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindOrAdd(InAssignment.TargetActor);

	if (InAssignment.CombatRole == ECombatRole::Engage)
	{
		if (targetSlotState.EngageCount >= GetEngageAssignmentTotalCap()) return false;

		if (InAssignment.EngageAdmission == EEngageAdmissionKind::GeneralBase)
		{
			if (targetSlotState.GeneralBaseEngageCount >= GetEngageAssignmentEngageCap()) return false;
			++targetSlotState.GeneralBaseEngageCount;
		}
		else if (InAssignment.EngageAdmission == EEngageAdmissionKind::HitReactiveExtra)
		{
			if (targetSlotState.HitReactiveExtraEngageCount >= GetEngageAssignmentHitReactiveExtraCap()) return false;
			++targetSlotState.HitReactiveExtraEngageCount;
		}
		else
		{
			return false;
		}

		++targetSlotState.EngageCount;
		return true;
	}

	if (InAssignment.CombatRole == ECombatRole::Alert)
	{
		if (targetSlotState.AlertCount >= GetEngageAssignmentAlertCap()) return false;

		++targetSlotState.AlertCount;
		return true;
	}

	if (InAssignment.CombatRole == ECombatRole::Observe)
	{
		if (targetSlotState.ObserveCount >= GetEngageAssignmentObserveCap()) return false;

		++targetSlotState.ObserveCount;
		return true;
	}

	return false;
}

// Assignment Retention Support

bool UCWorldSubsystem_CombatParticipation::CanRetainAssignment(const ACAIController* InCAIController, const FEngageAssignmentContext& InAssignment) const
{
	return InAssignment.IsValidAssignment()
		&& (HasActiveEvidenceForParticipantTarget(InCAIController, InAssignment.TargetActor)
			|| IsAssignmentLockActive(InCAIController, InAssignment));
}

bool UCWorldSubsystem_CombatParticipation::IsAssignmentLockActive(const ACAIController* InParticipant, const FEngageAssignmentContext& InAssignment) const
{
	const FCombatParticipationAssignmentLock* assignmentLock = AssignmentLockByParticipant.Find(InParticipant);
	if (!assignmentLock || !assignmentLock->Matches(InAssignment)) return false;
	if (!IsCombatParticipationTargetValid(InParticipant, assignmentLock->TargetActor)) return false;

	const UWorld* world = GetWorld();
	return IsValid(world) && world->GetTimeSeconds() < assignmentLock->ExpireTimeSeconds;
}

// -----------------------------------------------------------------------------
// Target Lifecycle
// -----------------------------------------------------------------------------

// Lifecycle Binding

void UCWorldSubsystem_CombatParticipation::BindParticipationTargetLifecycle(AActor* InTarget)
{
	BindParticipationTargetEndPlay(InTarget);
	BindParticipationTargetDeadState(InTarget);
}

void UCWorldSubsystem_CombatParticipation::BindParticipationTargetEndPlay(AActor* InTarget)
{
	if (!IsValid(InTarget) || EvidenceTargetEndPlayBindings.Contains(InTarget)) return;

	InTarget->OnEndPlay.AddDynamic(this, &UCWorldSubsystem_CombatParticipation::HandleEvidenceTargetEndPlay);
	EvidenceTargetEndPlayBindings.Add(InTarget);
}

void UCWorldSubsystem_CombatParticipation::BindParticipationTargetDeadState(AActor* InTarget)
{
	UCHealthComponent* healthComp = IsValid(InTarget) ? InTarget->FindComponentByClass<UCHealthComponent>() : nullptr;
	if (!IsValid(healthComp) || EvidenceTargetHealthBindings.Contains(healthComp)) return;

	healthComp->OnDeadStateChanged.AddUObject(this, &UCWorldSubsystem_CombatParticipation::HandleEvidenceTargetDeadStateChanged, healthComp);
	EvidenceTargetHealthBindings.Add(healthComp);
}

void UCWorldSubsystem_CombatParticipation::UnbindUnusedCombatParticipationTargetLifecycle()
{
	for (auto iterator = EvidenceTargetEndPlayBindings.CreateIterator(); iterator; ++iterator)
	{
		AActor* targetActor = *iterator;
		bool bHasTargetReference = false;
		for (const TPair<FCombatParticipationEvidenceKey, FCombatParticipationEvidence>& pair : EvidenceRegistry)
		{
			if (pair.Value.TargetActor == targetActor)
			{
				bHasTargetReference = true;
				break;
			}
		}

		if (!bHasTargetReference)
		{
			for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : AssignmentByParticipant)
			{
				if (pair.Value.TargetActor == targetActor)
				{
					bHasTargetReference = true;
					break;
				}
			}
		}

		if (!bHasTargetReference)
		{
			for (const TPair<ACAIController*, FCombatParticipationAssignmentLock>& pair : AssignmentLockByParticipant)
			{
				if (pair.Value.TargetActor == targetActor)
				{
					bHasTargetReference = true;
					break;
				}
			}
		}

		if (bHasTargetReference) continue;
		if (IsValid(targetActor))
		{
			targetActor->OnEndPlay.RemoveDynamic(this, &UCWorldSubsystem_CombatParticipation::HandleEvidenceTargetEndPlay);

			if (UCHealthComponent* healthComp = targetActor->FindComponentByClass<UCHealthComponent>())
			{
				healthComp->OnDeadStateChanged.RemoveAll(this);
				EvidenceTargetHealthBindings.Remove(healthComp);
			}
		}
		iterator.RemoveCurrent();
	}
}

// Target State Release

void UCWorldSubsystem_CombatParticipation::ReleaseTargetParticipationState(AActor* InTarget)
{
	if (!InTarget) return;

	for (auto iterator = EvidenceRegistry.CreateIterator(); iterator; ++iterator)
	{
		if (iterator.Value().TargetActor == InTarget)
		{
			iterator.RemoveCurrent();
		}
	}

	for (auto iterator = AssignmentLockByParticipant.CreateIterator(); iterator; ++iterator)
	{
		if (iterator.Value().TargetActor == InTarget)
		{
			iterator.RemoveCurrent();
		}
	}

	if (IsValid(InTarget))
	{
		InTarget->OnEndPlay.RemoveDynamic(this, &UCWorldSubsystem_CombatParticipation::HandleEvidenceTargetEndPlay);
	}
	EvidenceTargetEndPlayBindings.Remove(InTarget);
	if (UCHealthComponent* healthComp = InTarget->FindComponentByClass<UCHealthComponent>())
	{
		healthComp->OnDeadStateChanged.RemoveAll(this);
		EvidenceTargetHealthBindings.Remove(healthComp);
	}
	RebuildAssignments();
}

// Lifecycle Callback

void UCWorldSubsystem_CombatParticipation::HandleEvidenceTargetEndPlay(AActor* InTarget, EEndPlayReason::Type InEndPlayReason)
{
	ReleaseTargetParticipationState(InTarget);
}

void UCWorldSubsystem_CombatParticipation::HandleEvidenceTargetDeadStateChanged(const EDeadState InPreviousState, const EDeadState InNewState, UCHealthComponent* InHealthComponent)
{
	if (InNewState != EDeadState::Dead) return;

	AActor* targetActor = IsValid(InHealthComponent) ? InHealthComponent->GetOwner() : nullptr;
	if (!IsValid(targetActor)) return;

	ReleaseTargetParticipationState(targetActor);
}

// -----------------------------------------------------------------------------
// Runtime Cleanup
// -----------------------------------------------------------------------------

void UCWorldSubsystem_CombatParticipation::ClearCombatParticipationRuntimeState()
{
	ElapsedTime = 0.f;
	AssignmentWarmupStartTime = CCombatEngageConstants::UnsetAssignmentWarmupStartTime;
	bAssignmentWarmupCompleted = false;
	AssignmentRebuildId = CCombatEngageConstants::InitialAssignmentRebuildId;
	AssignmentRevisionSerial = 0;
	EvidenceRegistry.Reset();
	SuppressedParticipants.Reset();
	AssignmentLockByParticipant.Reset();
	for (AActor* targetActor : EvidenceTargetEndPlayBindings)
	{
		if (IsValid(targetActor)) targetActor->OnEndPlay.RemoveDynamic(this, &UCWorldSubsystem_CombatParticipation::HandleEvidenceTargetEndPlay);
	}
	EvidenceTargetEndPlayBindings.Reset();
	for (UCHealthComponent* healthComp : EvidenceTargetHealthBindings)
	{
		if (IsValid(healthComp)) healthComp->OnDeadStateChanged.RemoveAll(this);
	}
	EvidenceTargetHealthBindings.Reset();
	AssignmentByParticipant.Reset();
}
