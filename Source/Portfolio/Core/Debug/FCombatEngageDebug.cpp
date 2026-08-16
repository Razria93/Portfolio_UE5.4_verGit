#include "Core/Debug/FCombatEngageDebug.h"

#include "Core/Debug/FLog.h"
#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarEngageAssignmentAudit(
		TEXT("Portfolio.AI.RuntimeLOD.EngageAssignmentAudit"),
		0,
		TEXT("Print CombatEngage assignment warmup and rebuild summary audit logs. 0: disabled, 1: enabled."),
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

// Assignment Diagnostic Hook

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

void FCombatEngageDebug::RecordEngageAssignmentRebuildSummaryForAudit(int32 InRebuildId, const FEngageAssignmentRebuildDebugState& InDebugState, const TMap<ACAIController*, FEngageAssignmentContext>& InAssignments, int32 InGeneralBaseCap, int32 InHitReactiveExtraCap, int32 InTotalEngageCap, int32 InAlertCap, int32 InObserveCap)
{
	if (!ShouldAuditEngageAssignment()) return;

	int32 engageCount = 0;
	int32 alertCount = 0;
	int32 observeCount = 0;
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

		case ECombatRole::Observe:
			++observeCount;
			break;

		case ECombatRole::None:
		default:
			++noneCount;
			break;
		}
	}

	FLog::Log(FString::Printf(
		TEXT("[AI|CombatParticipation|AssignmentRebuildSummary] RebuildId=%d | GeneralBaseCap=%d | HitReactiveExtraCap=%d | TotalEngageCap=%d | AlertCap=%d | ObserveCap=%d | Candidates=%d | PreservedEngage=%d | PromotedAlertToEngage=%d | PromotedObserveToEngage=%d | FreshEngage=%d | PreservedAlert=%d | PromotedObserveToAlert=%d | FreshAlert=%d | PreservedObserve=%d | FreshObserve=%d | FinalEngage=%d | FinalAlert=%d | FinalObserve=%d | FinalNone=%d | FinalTotal=%d"),
		InRebuildId,
		InGeneralBaseCap,
		InHitReactiveExtraCap,
		InTotalEngageCap,
		InAlertCap,
		InObserveCap,
		InDebugState.CandidateCount,
		InDebugState.PreservedEngageCount,
		InDebugState.PromotedAlertToEngageCount,
		InDebugState.PromotedObserveToEngageCount,
		InDebugState.FreshEngageCount,
		InDebugState.PreservedAlertCount,
		InDebugState.PromotedObserveToAlertCount,
		InDebugState.FreshAlertCount,
		InDebugState.PreservedObserveCount,
		InDebugState.FreshObserveCount,
		engageCount,
		alertCount,
		observeCount,
		noneCount,
		InAssignments.Num()));
}
