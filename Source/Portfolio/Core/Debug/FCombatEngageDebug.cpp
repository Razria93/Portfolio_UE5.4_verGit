#include "Core/Debug/FCombatEngageDebug.h"
#include "Core/Debug/FLog.h"

#include "Controller/CAIController.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarEngageAssignmentAudit(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAudit"),
		0,
		TEXT("Print CombatEngage assignment warmup and rebuild summary audit logs. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarEngageAssignmentVerboseAudit(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentVerboseAudit"),
		0,
		TEXT("Print detailed CombatEngage assignment request candidate and slot decision audit logs. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif
}

// Gate

bool FCombatEngageDebug::ShouldAuditEngageAssignment()
{
#if !UE_BUILD_SHIPPING
	return CVarEngageAssignmentAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FCombatEngageDebug::ShouldAuditEngageAssignmentVerbose()
{
#if !UE_BUILD_SHIPPING
	return CVarEngageAssignmentVerboseAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

// Assignment Diagnostic Hook

void FCombatEngageDebug::RecordFreshEngageAssignmentAppliedForAudit(const FEngageRequestContext& InRequestContext, int32 InIndex, ECombatRole InCombatRole, const FEngageAssignmentSlotState& InSlotState, int32 InEngageCap, int32 InAlertCap)
{
	if (!ShouldAuditEngageAssignmentVerbose()) return;

	const APawn* controlledPawn = IsValid(InRequestContext.RequestController) ? InRequestContext.RequestController->GetPawn() : nullptr;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatEngage|FreshAssignmentApplied] Controller=%s | Pawn=%s | Target=%s | Priority=%d | Index=%d | Distance=%.3f | CombatRole=%s | EngageSlot=%d/%d | AlertSlot=%d/%d"),
		*GetNameSafe(InRequestContext.RequestController),
		*GetNameSafe(controlledPawn),
		*GetNameSafe(InRequestContext.TargetActor),
		InRequestContext.TargetPriority,
		InIndex,
		InRequestContext.DistanceToTarget,
		*UEnum::GetValueAsString(InCombatRole),
		InSlotState.EngageCount,
		InEngageCap,
		InSlotState.AlertCount,
		InAlertCap));
}

void FCombatEngageDebug::RecordEngageAssignmentPromotedForAudit(const FEngageRequestContext& InRequestContext, const FEngageAssignmentSlotState& InSlotState, int32 InEngageCap, int32 InAlertCap)
{
	if (!ShouldAuditEngageAssignmentVerbose()) return;

	const APawn* controlledPawn = IsValid(InRequestContext.RequestController) ? InRequestContext.RequestController->GetPawn() : nullptr;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatEngage|AssignmentPromoted] Controller=%s | Pawn=%s | Target=%s | Priority=%d | Distance=%.3f | PreviousRole=%s | CombatRole=%s | EngageSlot=%d/%d | AlertSlot=%d/%d"),
		*GetNameSafe(InRequestContext.RequestController),
		*GetNameSafe(controlledPawn),
		*GetNameSafe(InRequestContext.TargetActor),
		InRequestContext.TargetPriority,
		InRequestContext.DistanceToTarget,
		*UEnum::GetValueAsString(ECombatRole::Alert),
		*UEnum::GetValueAsString(ECombatRole::Engage),
		InSlotState.EngageCount,
		InEngageCap,
		InSlotState.AlertCount,
		InAlertCap));
}

void FCombatEngageDebug::RecordEngageAssignmentPreservedForAudit(const ACAIController* InAIController, const FEngageAssignmentContext& InAssignment, const FEngageAssignmentSlotState& InSlotState, float InLeaseAge, float InLeaseRemaining, int32 InEngageCap, int32 InAlertCap)
{
	if (!ShouldAuditEngageAssignmentVerbose()) return;

	const APawn* controlledPawn = IsValid(InAIController) ? InAIController->GetPawn() : nullptr;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatEngage|AssignmentPreserved] Controller=%s | Pawn=%s | Target=%s | CombatRole=%s | LeaseAge=%.3f | LeaseRemaining=%.3f | EngageSlot=%d/%d | AlertSlot=%d/%d"),
		*GetNameSafe(InAIController),
		*GetNameSafe(controlledPawn),
		*GetNameSafe(InAssignment.TargetActor),
		*UEnum::GetValueAsString(InAssignment.CombatRole),
		InLeaseAge,
		InLeaseRemaining,
		InSlotState.EngageCount,
		InEngageCap,
		InSlotState.AlertCount,
		InAlertCap));
}

void FCombatEngageDebug::RecordEngageAssignmentWarmupDelayedForAudit(int32 InRebuildId, int32 InRequestCount, float InWarmupElapsedTime, float InWarmupTime)
{
	if (!ShouldAuditEngageAssignment()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatEngage|AssignmentWarmupDelayed] RebuildId=%d | RequestCount=%d | WarmupElapsed=%.3f | WarmupTime=%.3f"),
		InRebuildId,
		InRequestCount,
		InWarmupElapsedTime,
		InWarmupTime));
}

void FCombatEngageDebug::RecordEngageRequestSnapshotForAudit(int32 InRebuildId, const TMap<ACAIController*, FEngageRequestContext>& InRequestSnapshot, const TMap<AActor*, TArray<FEngageRequestContext>>& InRequestBucket)
{
	if (!ShouldAuditEngageAssignmentVerbose()) return;

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatEngage|RequestSnapshot] RebuildId=%d | RequestCount=%d | TargetBucketCount=%d"),
		InRebuildId,
		InRequestSnapshot.Num(),
		InRequestBucket.Num()));

	for (const TPair<AActor*, TArray<FEngageRequestContext>>& pair : InRequestBucket)
	{
		AActor* targetActor = pair.Key;
		TArray<FEngageRequestContext> requestContexts = pair.Value;
		requestContexts.Sort([](const FEngageRequestContext& A, const FEngageRequestContext& B)
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

		FLog::Log(FString::Printf(
			TEXT("[AI|CombatEngage|RequestBucket] RebuildId=%d | Target=%s | CandidateCount=%d"),
			InRebuildId,
			*GetNameSafe(targetActor),
			requestContexts.Num()));

		for (int32 i = 0; i < requestContexts.Num(); ++i)
		{
			const FEngageRequestContext& requestContext = requestContexts[i];
			const APawn* controlledPawn = IsValid(requestContext.RequestController) ? requestContext.RequestController->GetPawn() : nullptr;

			FLog::Log(FString::Printf(
				TEXT("[AI|CombatEngage|RequestCandidate] RebuildId=%d | Target=%s | Index=%d | Controller=%s | Pawn=%s | Priority=%d | WasEngaged=%s | Distance=%.3f"),
				InRebuildId,
				*GetNameSafe(targetActor),
				i,
				*GetNameSafe(requestContext.RequestController),
				*GetNameSafe(controlledPawn),
				requestContext.TargetPriority,
				requestContext.bWasEngaged ? TEXT("true") : TEXT("false"),
				requestContext.DistanceToTarget));
		}
	}
}

void FCombatEngageDebug::RecordEngageAssignmentRebuildSummaryForAudit(int32 InRebuildId, const FEngageAssignmentRebuildDebugState& InDebugState, const TMap<ACAIController*, FEngageAssignmentContext>& InAssignments, int32 InEngageCap, int32 InAlertCap)
{
	if (!ShouldAuditEngageAssignment()) return;

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

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatEngage|AssignmentRebuildSummary] RebuildId=%d | EngageCap=%d | AlertCap=%d | RequestSnapshot=%d | TargetBuckets=%d | WarmupRequest=%d | FreshApplied=%d | Promoted=%d | PreservedEngage=%d | PreservedAlert=%d | FinalEngage=%d | FinalAlert=%d | FinalNone=%d | FinalTotal=%d"),
		InRebuildId,
		InEngageCap,
		InAlertCap,
		InDebugState.RequestSnapshotCount,
		InDebugState.RequestBucketCount,
		InDebugState.WarmupRequestCount,
		InDebugState.FreshAppliedCount,
		InDebugState.PromotedCount,
		InDebugState.PreservedEngageCount,
		InDebugState.PreservedAlertCount,
		engageCount,
		alertCount,
		noneCount,
		InAssignments.Num()));
}
