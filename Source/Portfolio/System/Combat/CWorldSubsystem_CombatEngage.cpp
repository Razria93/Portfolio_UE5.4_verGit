#include "System/Combat/CWorldSubsystem_CombatEngage.h"
#include "ProjectGlobal.h"
#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "AIController.h"

#include "Controller/CAIController.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"
#include "Core/Profiling/CCombatFeedbackProfiling.h"

#include "Type/CWorldSubSystemStructure.h"

namespace
{
	TAutoConsoleVariable<float> CVarEngageAssignmentWarmupTime(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentWarmupTime"),
		0.0f,
		TEXT("Delays the first CombatEngage assignment rebuild until request candidates are warmed up. 0: disabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentAudit(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAudit"),
		0,
		TEXT("Print minimal CombatEngage assignment warmup audit logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentVerboseAudit(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit"),
		0,
		TEXT("Print detailed CombatEngage assignment candidate logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentEngageCap(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentEngageCap"),
		2,
		TEXT("Controls max Engage assignees per target for AI Runtime LOD profiling. Default: 2."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentAlertCap(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAlertCap"),
		6,
		TEXT("Controls max Alert assignees per target for AI Runtime LOD profiling. Default: 6."),
		ECVF_Default);

	float GetEngageAssignmentWarmupTime()
	{
		return FMath::Max(0.f, CVarEngageAssignmentWarmupTime.GetValueOnGameThread());
	}

	int32 GetEngageAssignmentEngageCap()
	{
		return FMath::Max(0, CVarEngageAssignmentEngageCap.GetValueOnGameThread());
	}

	int32 GetEngageAssignmentAlertCap()
	{
		return FMath::Max(0, CVarEngageAssignmentAlertCap.GetValueOnGameThread());
	}

	bool ShouldPrintEngageAssignmentAudit()
	{
		return CVarEngageAssignmentAudit.GetValueOnGameThread() != 0;
	}

	bool ShouldPrintEngageAssignmentVerboseAudit()
	{
		return CVarEngageAssignmentVerboseAudit.GetValueOnGameThread() != 0;
	}
}

void UCWorldSubsystem_CombatEngage::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCWorldSubsystem_CombatEngage::Deinitialize()
{
	ClearEngageRuntimeState();

	Super::Deinitialize();
}

void UCWorldSubsystem_CombatEngage::Tick(float DeltaTime)
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_CombatEngage_Tick);
	FCombatCollisionProfilingCounters::FlushToCsv();
	FCombatFeedbackProfiling::FlushToCsv();

	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;
	if (ElapsedTime < RebuildInterval) return;

	ElapsedTime = 0.f;
	RebuildAssignments();
}

TStatId UCWorldSubsystem_CombatEngage::GetStatId() const
{
	// [NOTE] Returns a stat id, so Unreal can track this tickable subsystem in the unreal stat system.
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCWorldSubsystem_CombatEngage, STATGROUP_Tickables);
}

// Query

FEngageAssignmentContext UCWorldSubsystem_CombatEngage::GetAssignment(const ACAIController* InCAIController) const
{
	if (!IsValid(InCAIController)) return FEngageAssignmentContext();

	const FEngageAssignmentContext* found = AssignmentContainer.Find(InCAIController);
	if(!found) return FEngageAssignmentContext();

	return *found;
}

// Request

void UCWorldSubsystem_CombatEngage::SubmitRequest(const FEngageRequestContext & InEngageRequestContext)
{
	if (!IsValid(InEngageRequestContext.RequestController)) return;

	StartAssignmentWarmupIfNeeded();

	// Override Request
	RequestContainer.FindOrAdd(InEngageRequestContext.RequestController) = InEngageRequestContext;

	UWorld* world = GetWorld();
	LastRequestTimeContainer.FindOrAdd(InEngageRequestContext.RequestController) = IsValid(world) ? world->GetTimeSeconds() : 0.f;
}

// Assignment

void UCWorldSubsystem_CombatEngage::RebuildAssignments()
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_CombatEngage_RebuildAssignments);

	++AssignmentRebuildId;

	// Delay for Warmup
	if (ShouldDelayAssignmentForWarmup())
	{
		if (ShouldPrintEngageAssignmentAudit())
		{
			PrintAssignmentWarmupDelay(AssignmentRebuildId);
		}

		return;
	}

	bool bCompletedWarmupThisRebuild = false;

	// Flag Toogle
	if (!bAssignmentWarmupCompleted)
	{
		if (GetEngageAssignmentWarmupTime() > 0.f && AssignmentWarmupStartTime < 0.f) return;

		bAssignmentWarmupCompleted = true;
		bCompletedWarmupThisRebuild = true;
	}

	TMap<ACAIController*, FEngageAssignmentContext> nextAssignments;
	TMap<AActor*, FEngageAssignmentSlotState> slotState;
	FEngageAssignmentRebuildDebugState rebuildDebugState;

	TMap<ACAIController*, FEngageRequestContext> requestSnapshot = ConsumeRequestSnapshot();
	TMap<AActor*, TArray<FEngageRequestContext>> requestBucket;
	BuildRequestBucket(requestSnapshot, requestBucket);

	rebuildDebugState.WarmupRequestCount = requestSnapshot.Num();
	rebuildDebugState.RequestSnapshotCount = requestSnapshot.Num();
	rebuildDebugState.RequestBucketCount = requestBucket.Num();

	if (ShouldPrintEngageAssignmentVerboseAudit())
	{
		PrintEngageRequestSnapshot(AssignmentRebuildId, requestSnapshot, requestBucket);
	}

	PreserveExistingEngageAssignments(nextAssignments, slotState, rebuildDebugState);
	PromoteExistingAlertAssignments(requestBucket, nextAssignments, slotState, rebuildDebugState);
	PreserveExistingAlertAssignments(nextAssignments, slotState, rebuildDebugState);
	ApplyFreshRequestAssignments(requestBucket, nextAssignments, slotState, rebuildDebugState);

	if (ShouldPrintEngageAssignmentAudit() && (bCompletedWarmupThisRebuild || AssignmentRebuildId == 1))
	{
		PrintEngageAssignmentRebuildSummary(AssignmentRebuildId, rebuildDebugState, nextAssignments);
	}

	AssignmentContainer = MoveTemp(nextAssignments);
}

TMap<ACAIController*, FEngageRequestContext> UCWorldSubsystem_CombatEngage::ConsumeRequestSnapshot()
{
	TMap<ACAIController*, FEngageRequestContext> requestSnapshot = MoveTemp(RequestContainer);
	RequestContainer.Reset();

	return requestSnapshot;
}

void UCWorldSubsystem_CombatEngage::BuildRequestBucket(const TMap<ACAIController*, FEngageRequestContext>& InRequestSnapshot, TMap<AActor*, TArray<FEngageRequestContext>>& OutRequestBucket) const
{
	OutRequestBucket.Reset();

	for (const TPair<ACAIController*, FEngageRequestContext>& pair : InRequestSnapshot)
	{
		const FEngageRequestContext& request = pair.Value;

		if (!IsValid(request.RequestController) || !IsValid(request.TargetActor)) continue;

		OutRequestBucket.FindOrAdd(request.TargetActor).Add(request);
	}
}

void UCWorldSubsystem_CombatEngage::SortRequestContexts(TArray<FEngageRequestContext>& InOutRequestContexts) const
{
	InOutRequestContexts.Sort([](const FEngageRequestContext& A, const FEngageRequestContext& B)
		{
			if (A.TargetPriority != B.TargetPriority)
			{
				return A.TargetPriority < B.TargetPriority;
			}

			if (A.bWasEngaged != B.bWasEngaged)
			{
				return A.bWasEngaged;
			}

			return A.DistanceToTarget < B.DistanceToTarget;
		});
}

void UCWorldSubsystem_CombatEngage::StartAssignmentWarmupIfNeeded()
{
	if (bAssignmentWarmupCompleted) return;
	if (AssignmentWarmupStartTime >= 0.f) return;
	if (GetEngageAssignmentWarmupTime() <= 0.f) return;

	const UWorld* world = GetWorld();
	AssignmentWarmupStartTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
}

bool UCWorldSubsystem_CombatEngage::ShouldDelayAssignmentForWarmup() const
{
	if (bAssignmentWarmupCompleted) return false;
	if (AssignmentWarmupStartTime < 0.f) return false;
	if (GetEngageAssignmentWarmupTime() <= 0.f) return false;

	return GetAssignmentWarmupElapsedTime() < GetEngageAssignmentWarmupTime();
}

float UCWorldSubsystem_CombatEngage::GetAssignmentWarmupElapsedTime() const
{
	if (AssignmentWarmupStartTime < 0.f) return 0.f;

	const UWorld* world = GetWorld();
	if (!IsValid(world)) return 0.f;

	return FMath::Max(0.f, world->GetTimeSeconds() - AssignmentWarmupStartTime);
}

void UCWorldSubsystem_CombatEngage::PreserveExistingEngageAssignments(TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const
{
	for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : AssignmentContainer)
	{
		ACAIController* aiController = pair.Key;
		const FEngageAssignmentContext& previousAssignment = pair.Value;

		if (!IsValid(aiController)) continue;

		if (!previousAssignment.IsValidAssignment()) continue;
		if (previousAssignment.CombatRole != ECombatRole::Engage) continue;

		if (!IsAssignmentLeaseValid(aiController)) continue;

		if (!TryReserveAssignmentSlot(previousAssignment, InOutSlotState)) continue;

		InOutNextAssignments.Add(aiController, previousAssignment);
		++InOutDebugState.PreservedEngageCount;

		const FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindChecked(previousAssignment.TargetActor);
		// PrintPreservedAssignment(aiController, previousAssignment, targetSlotState);
	}
}

void UCWorldSubsystem_CombatEngage::PromoteExistingAlertAssignments(const TMap<AActor*, TArray<FEngageRequestContext>>& InRequestBucket, TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const
{
	for (const TPair<AActor*, TArray<FEngageRequestContext>>& pair : InRequestBucket)
	{
		AActor* targetActor = pair.Key;
		if (!IsValid(targetActor)) continue;

		TArray<FEngageRequestContext> requestContexts = pair.Value;
		SortRequestContexts(requestContexts);

		for (int32 i = 0; i < requestContexts.Num(); ++i)
		{
			if (i >= GetEngageAssignmentEngageCap()) break;

			ACAIController* requestController = requestContexts[i].RequestController;
			if (!IsValid(requestController)) continue;
			if (InOutNextAssignments.Contains(requestController)) continue;

			const FEngageAssignmentContext* previousAssignment = AssignmentContainer.Find(requestController);
			if (!previousAssignment) continue;
			if (!previousAssignment->IsValidAssignment()) continue;
			if (previousAssignment->CombatRole != ECombatRole::Alert) continue;

			if (!IsAssignmentLeaseValid(requestController)) continue;

			FEngageAssignmentContext promotedAssignment;
			promotedAssignment.TargetActor = targetActor;
			promotedAssignment.CombatRole = ECombatRole::Engage;

			if (!TryReserveAssignmentSlot(promotedAssignment, InOutSlotState)) continue;

			InOutNextAssignments.Add(requestController, promotedAssignment);
			++InOutDebugState.PromotedCount;

			const FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindChecked(targetActor);
			// PrintPromotedEngageAssignment(requestContexts[i], targetSlotState);
		}
	}
}

void UCWorldSubsystem_CombatEngage::PreserveExistingAlertAssignments(TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const
{
	for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : AssignmentContainer)
	{
		ACAIController* aiController = pair.Key;
		const FEngageAssignmentContext& previousAssignment = pair.Value;

		if (!IsValid(aiController)) continue;
		if (InOutNextAssignments.Contains(aiController)) continue;

		if (!previousAssignment.IsValidAssignment()) continue;
		if (previousAssignment.CombatRole != ECombatRole::Alert) continue;

		if (!IsAssignmentLeaseValid(aiController)) continue;

		if (!TryReserveAssignmentSlot(previousAssignment, InOutSlotState)) continue;

		InOutNextAssignments.Add(aiController, previousAssignment);
		++InOutDebugState.PreservedAlertCount;

		const FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindChecked(previousAssignment.TargetActor);
		// PrintPreservedAssignment(aiController, previousAssignment, targetSlotState);
	}
}

void UCWorldSubsystem_CombatEngage::ApplyFreshRequestAssignments(const TMap<AActor*, TArray<FEngageRequestContext>>& InRequestBucket, TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const
{
	for (const TPair<AActor*, TArray<FEngageRequestContext>>& pair : InRequestBucket)
	{
		AActor* targetActor = pair.Key;
		if (!IsValid(targetActor)) continue;

		TArray<FEngageRequestContext> requestContexts = pair.Value;
		SortRequestContexts(requestContexts);

		for (int32 i = 0; i < requestContexts.Num(); ++i)
		{
			ACAIController* requestController = requestContexts[i].RequestController;
			if (!IsValid(requestController)) continue;
			if (InOutNextAssignments.Contains(requestController)) continue;

			FEngageAssignmentContext freshAssignment;
			freshAssignment.TargetActor = targetActor;

			FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindOrAdd(targetActor);
			if (targetSlotState.EngageCount < GetEngageAssignmentEngageCap())
			{
				freshAssignment.CombatRole = ECombatRole::Engage;
			}
			else if (targetSlotState.AlertCount < GetEngageAssignmentAlertCap())
			{
				freshAssignment.CombatRole = ECombatRole::Alert;
			}
			else
			{
				continue;
			}

			if (!TryReserveAssignmentSlot(freshAssignment, InOutSlotState)) continue;

			InOutNextAssignments.Add(requestController, freshAssignment);
			++InOutDebugState.FreshAppliedCount;

			const FEngageAssignmentSlotState& updatedSlotState = InOutSlotState.FindChecked(targetActor);
			// PrintAppliedFreshEngageAssignment(requestContexts[i], i, freshAssignment.CombatRole, updatedSlotState);
		}
	}
}

bool UCWorldSubsystem_CombatEngage::TryReserveAssignmentSlot(const FEngageAssignmentContext& InAssignment, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState) const
{
	if (!InAssignment.IsValidAssignment()) return false;

	FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindOrAdd(InAssignment.TargetActor);

	if (InAssignment.CombatRole == ECombatRole::Engage)
	{
		if (targetSlotState.EngageCount >= GetEngageAssignmentEngageCap()) return false;

		++targetSlotState.EngageCount;
		return true;
	}

	if (InAssignment.CombatRole == ECombatRole::Alert)
	{
		if (targetSlotState.AlertCount >= GetEngageAssignmentAlertCap()) return false;

		++targetSlotState.AlertCount;
		return true;
	}

	return false;
}

bool UCWorldSubsystem_CombatEngage::IsAssignmentLeaseValid(const ACAIController* InCAIController) const
{
	if (!IsValid(InCAIController)) return false;

	const float* lastRequestTime = LastRequestTimeContainer.Find(InCAIController);
	if (!lastRequestTime) return false;

	const UWorld* world = GetWorld();
	if (!IsValid(world)) return false;

	return world->GetTimeSeconds() - *lastRequestTime <= AssignmentLeaseDuration;
}

// Runtime State

void UCWorldSubsystem_CombatEngage::ClearEngageRuntimeState()
{
	ElapsedTime = 0.f;
	AssignmentWarmupStartTime = -1.f;
	bAssignmentWarmupCompleted = false;
	AssignmentRebuildId = 0;
	RequestContainer.Reset();
	LastRequestTimeContainer.Reset();
	AssignmentContainer.Reset();
}

// Debug

void UCWorldSubsystem_CombatEngage::PrintAppliedFreshEngageAssignment(const FEngageRequestContext& InRequestContext, const int& InIndex, const ECombatRole& InCombatRole, const FEngageAssignmentSlotState& InSlotState) const
{
	const APawn* controlledPawn = IsValid(InRequestContext.RequestController) ? InRequestContext.RequestController->GetPawn() : nullptr;

	FLog::Log(TEXT("==== AppliedFreshEngageAssignment ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AIController"), *GetNameSafe(InRequestContext.RequestController)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ControlledPawn"), *GetNameSafe(controlledPawn)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InRequestContext.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Priority"), InRequestContext.TargetPriority));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Index"), InIndex));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("DistanceToTarget"), InRequestContext.DistanceToTarget));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CombatRole"), *UEnum::GetValueAsString(InCombatRole)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d / %d"), TEXT("EngageSlot"), InSlotState.EngageCount, GetEngageAssignmentEngageCap()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d / %d"), TEXT("AlertSlot"), InSlotState.AlertCount, GetEngageAssignmentAlertCap()));
	FLog::Log(TEXT("===================================="));
}

void UCWorldSubsystem_CombatEngage::PrintPromotedEngageAssignment(const FEngageRequestContext& InRequestContext, const FEngageAssignmentSlotState& InSlotState) const
{
	const APawn* controlledPawn = IsValid(InRequestContext.RequestController) ? InRequestContext.RequestController->GetPawn() : nullptr;

	FLog::Log(TEXT("==== PromotedEngageAssignment ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AIController"), *GetNameSafe(InRequestContext.RequestController)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ControlledPawn"), *GetNameSafe(controlledPawn)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InRequestContext.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Priority"), InRequestContext.TargetPriority));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("DistanceToTarget"), InRequestContext.DistanceToTarget));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("PreviousRole"), *UEnum::GetValueAsString(ECombatRole::Alert)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CombatRole"), *UEnum::GetValueAsString(ECombatRole::Engage)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d / %d"), TEXT("EngageSlot"), InSlotState.EngageCount, GetEngageAssignmentEngageCap()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d / %d"), TEXT("AlertSlot"), InSlotState.AlertCount, GetEngageAssignmentAlertCap()));
	FLog::Log(TEXT("=================================="));
}

void UCWorldSubsystem_CombatEngage::PrintPreservedAssignment(const ACAIController* InCAIController, const FEngageAssignmentContext& InAssignment, const FEngageAssignmentSlotState& InSlotState) const
{
	const APawn* controlledPawn = IsValid(InCAIController) ? InCAIController->GetPawn() : nullptr;
	const float* lastRequestTime = LastRequestTimeContainer.Find(InCAIController);

	const UWorld* world = GetWorld();
	const float currentTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	const float leaseAge = lastRequestTime ? currentTime - *lastRequestTime : -1.f;
	const float leaseRemaining = lastRequestTime ? FMath::Max(0.f, AssignmentLeaseDuration - leaseAge) : 0.f;

	FLog::Log(TEXT("==== PreservedAssignment ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AIController"), *GetNameSafe(InCAIController)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ControlledPawn"), *GetNameSafe(controlledPawn)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InAssignment.TargetActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CombatRole"), *UEnum::GetValueAsString(InAssignment.CombatRole)));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("LeaseAge"), leaseAge));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("LeaseRemaining"), leaseRemaining));
	FLog::Log(FString::Printf(TEXT("%-20s: %d / %d"), TEXT("EngageSlot"), InSlotState.EngageCount, GetEngageAssignmentEngageCap()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d / %d"), TEXT("AlertSlot"), InSlotState.AlertCount, GetEngageAssignmentAlertCap()));
	FLog::Log(TEXT("============================="));
}

void UCWorldSubsystem_CombatEngage::PrintAssignmentWarmupDelay(const int& InRebuildId) const
{
	FLog::Log(FString::Printf(
		TEXT("[EngageAssignmentWarmupDelay] RebuildId=%d | RequestCount=%d | WarmupElapsed=%.3f | WarmupTime=%.3f"),
		InRebuildId,
		RequestContainer.Num(),
		GetAssignmentWarmupElapsedTime(),
		GetEngageAssignmentWarmupTime()));
}

void UCWorldSubsystem_CombatEngage::PrintEngageRequestSnapshot(const int& InRebuildId, const TMap<ACAIController*, FEngageRequestContext>& InRequestSnapshot, const TMap<AActor*, TArray<FEngageRequestContext>>& InRequestBucket) const
{
	FLog::Log(TEXT("==== EngageRequestSnapshot ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("RebuildId"), InRebuildId));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("RequestCount"), InRequestSnapshot.Num()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("TargetBucketCount"), InRequestBucket.Num()));

	for (const TPair<AActor*, TArray<FEngageRequestContext>>& pair : InRequestBucket)
	{
		AActor* targetActor = pair.Key;
		TArray<FEngageRequestContext> requestContexts = pair.Value;
		SortRequestContexts(requestContexts);

		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(targetActor)));
		FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("CandidateCount"), requestContexts.Num()));

		for (int32 i = 0; i < requestContexts.Num(); ++i)
		{
			const FEngageRequestContext& requestContext = requestContexts[i];
			const APawn* controlledPawn = IsValid(requestContext.RequestController) ? requestContext.RequestController->GetPawn() : nullptr;

			FLog::Log(FString::Printf(
				TEXT("  [%02d] Controller=%s | Pawn=%s | Priority=%d | WasEngaged=%s | Distance=%.3f"),
				i,
				*GetNameSafe(requestContext.RequestController),
				*GetNameSafe(controlledPawn),
				requestContext.TargetPriority,
				requestContext.bWasEngaged ? TEXT("true") : TEXT("false"),
				requestContext.DistanceToTarget));
		}
	}

	FLog::Log(TEXT("==============================="));
}

void UCWorldSubsystem_CombatEngage::PrintEngageAssignmentRebuildSummary(const int& InRebuildId, const FEngageAssignmentRebuildDebugState& InDebugState, const TMap<ACAIController*, FEngageAssignmentContext>& InAssignments) const
{
	int32 engageCount = 0;
	int32 alertCount = 0;
	int32 noneCount = 0;

	for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : InAssignments)
	{
		switch (pair.Value.CombatRole)
		{
		case ECombatRole::Engage:
			++engageCount;
			break;

		case ECombatRole::Alert:
			++alertCount;
			break;

		case ECombatRole::None:
		default:
			++noneCount;
			break;
		}
	}

	FLog::Log(TEXT("==== EngageAssignmentRebuildSummary ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("RebuildId"), InRebuildId));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("EngageCap"), GetEngageAssignmentEngageCap()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("AlertCap"), GetEngageAssignmentAlertCap()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("RequestSnapshot"), InDebugState.RequestSnapshotCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("TargetBuckets"), InDebugState.RequestBucketCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("WarmupRequest"), InDebugState.WarmupRequestCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("FreshApplied"), InDebugState.FreshAppliedCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Promoted"), InDebugState.PromotedCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("PreservedEngage"), InDebugState.PreservedEngageCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("PreservedAlert"), InDebugState.PreservedAlertCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("FinalEngage"), engageCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("FinalAlert"), alertCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("FinalNone"), noneCount));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("FinalTotal"), InAssignments.Num()));
	FLog::Log(TEXT("========================================"));
}
