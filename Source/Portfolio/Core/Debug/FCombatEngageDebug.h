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

public:
	// Assignment Diagnostic Hook
	static void RecordEngageAssignmentWarmupDelayedForAudit(int32 InRebuildId, int32 InRequestCount, float InWarmupElapsedTime, float InWarmupTime);
	static void RecordEngageAssignmentRebuildSummaryForAudit(int32 InRebuildId, const FEngageAssignmentRebuildDebugState& InDebugState, const TMap<ACAIController*, FEngageAssignmentContext>& InAssignments, int32 InGeneralBaseCap, int32 InHitReactiveExtraCap, int32 InTotalEngageCap, int32 InAlertCap, int32 InObserveCap);
};
