#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FCombatEngageDebugTypes.h"
#include "Type/CEngageAssignmentTypes.h"

class AActor;
class ACAIController;

class PORTFOLIO_API FCombatEngageDebug
{
public:
	// Gate
	static bool ShouldAuditEngageAssignment();
	static bool ShouldAuditEngageAssignmentVerbose();

public:
	// Assignment Diagnostic Hook
	static void RecordFreshEngageAssignmentAppliedForAudit(const FEngageRequestContext& InRequestContext, int32 InIndex, ECombatRole InCombatRole, const FEngageAssignmentSlotState& InSlotState, int32 InEngageCap, int32 InAlertCap);
	static void RecordEngageAssignmentPromotedForAudit(const FEngageRequestContext& InRequestContext, const FEngageAssignmentSlotState& InSlotState, int32 InEngageCap, int32 InAlertCap);
	static void RecordEngageAssignmentPreservedForAudit(const ACAIController* InAIController, const FEngageAssignmentContext& InAssignment, const FEngageAssignmentSlotState& InSlotState, float InLeaseAge, float InLeaseRemaining, int32 InEngageCap, int32 InAlertCap);
	static void RecordEngageAssignmentWarmupDelayedForAudit(int32 InRebuildId, int32 InRequestCount, float InWarmupElapsedTime, float InWarmupTime);
	static void RecordEngageRequestSnapshotForAudit(int32 InRebuildId, const TMap<ACAIController*, FEngageRequestContext>& InRequestSnapshot, const TMap<AActor*, TArray<FEngageRequestContext>>& InRequestBucket);
	static void RecordEngageAssignmentRebuildSummaryForAudit(int32 InRebuildId, const FEngageAssignmentRebuildDebugState& InDebugState, const TMap<ACAIController*, FEngageAssignmentContext>& InAssignments, int32 InEngageCap, int32 InAlertCap);
};
