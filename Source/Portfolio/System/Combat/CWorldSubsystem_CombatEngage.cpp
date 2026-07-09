#include "System/Combat/CWorldSubsystem_CombatEngage.h"
#include "ProjectGlobal.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "AIController.h"

#include "Controller/CAIController.h"

#include "Type/CWorldSubSystemStructure.h"

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

EAIUpdatePrecision UCWorldSubsystem_CombatEngage::GetAIUpdatePrecision(const ACAIController* InCAIController) const
{
	if (!IsValid(InCAIController)) return EAIUpdatePrecision::High;

	const FEngageAssignmentContext* foundAssignment = AssignmentContainer.Find(InCAIController);
	if (foundAssignment && foundAssignment->CombatRole == ECombatRole::Engage) return EAIUpdatePrecision::High;
	if (foundAssignment && foundAssignment->CombatRole == ECombatRole::Alert) return EAIUpdatePrecision::Reduced;

	if (RequestContainer.Contains(InCAIController)) return EAIUpdatePrecision::Reduced;

	return EAIUpdatePrecision::Low;
}

// Request

void UCWorldSubsystem_CombatEngage::SubmitRequest(const FEngageRequestContext & InEngageRequestContext)
{
	if (!IsValid(InEngageRequestContext.RequestController)) return;

	// Override Request
	RequestContainer.FindOrAdd(InEngageRequestContext.RequestController) = InEngageRequestContext;

	UWorld* world = GetWorld();
	LastRequestTimeContainer.FindOrAdd(InEngageRequestContext.RequestController) = IsValid(world) ? world->GetTimeSeconds() : 0.f;
}

// Assignment

void UCWorldSubsystem_CombatEngage::RebuildAssignments()
{
	CSV_SCOPED_TIMING_STAT_GLOBAL(PortfolioAI_CombatEngage_RebuildAssignments);

	TMap<ACAIController*, FEngageAssignmentContext> nextAssignments;
	TSet<ACAIController*> freshRequestControllers;
	TMap<AActor*, FEngageAssignmentSlotState> slotState;

	TMap<ACAIController*, FEngageRequestContext> requestSnapshot = ConsumeRequestSnapshot();
	TMap<AActor*, TArray<FEngageRequestContext>> requestBucket;
	BuildRequestBucket(requestSnapshot, requestBucket);
	ApplyFreshRequestAssignments(requestBucket, nextAssignments, freshRequestControllers, slotState);
	PreserveLeasedAssignments(nextAssignments, freshRequestControllers, slotState);

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

void UCWorldSubsystem_CombatEngage::ApplyFreshRequestAssignments(const TMap<AActor*, TArray<FEngageRequestContext>>& InRequestBucket, TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TSet<ACAIController*>& OutFreshRequestControllers, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState) const
{
	for (const TPair<AActor*, TArray<FEngageRequestContext>>& pair : InRequestBucket)
	{
		AActor* targetActor = pair.Key;
		if (!IsValid(targetActor)) continue;

		TArray<FEngageRequestContext> requestContexts = pair.Value;
		SortRequestContexts(requestContexts);

		FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindOrAdd(targetActor);
		const int32 maxAssignedPerTarget = MaxEngagersPerTarget + MaxAlertersPerTarget;

		for (int32 i = 0; i < requestContexts.Num(); ++i)
		{
			ACAIController* requestController = requestContexts[i].RequestController;
			if (!IsValid(requestController)) continue;

			OutFreshRequestControllers.Add(requestController);
			if (i >= maxAssignedPerTarget) continue;

			const ECombatRole combatRole = i < MaxEngagersPerTarget ? ECombatRole::Engage : ECombatRole::Alert;

			FEngageAssignmentContext engageAssignmentContext;
			engageAssignmentContext.TargetActor = targetActor;
			engageAssignmentContext.CombatRole = combatRole;

			InOutNextAssignments.Add(requestController, engageAssignmentContext);

			if (combatRole == ECombatRole::Engage)
			{
				++targetSlotState.EngageCount;
			}
			else if (combatRole == ECombatRole::Alert)
			{
				++targetSlotState.AlertCount;
			}

			// PrintEngageContext(requestController, targetActor, requestContexts[i].TargetPriority, i, requestContexts[i].DistanceToTarget, combatRole);
		}
	}
}

void UCWorldSubsystem_CombatEngage::PreserveLeasedAssignments(TMap<ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, const TSet<ACAIController*>& InFreshRequestControllers, TMap<AActor*, FEngageAssignmentSlotState>& InOutSlotState) const
{
	for (const TPair<ACAIController*, FEngageAssignmentContext>& pair : AssignmentContainer)
	{
		ACAIController* aiController = pair.Key;
		const FEngageAssignmentContext& previousAssignment = pair.Value;

		if (!IsValid(aiController)) continue;
		if (InFreshRequestControllers.Contains(aiController)) continue;
		if (!previousAssignment.IsValidAssignment()) continue;
		if (!IsAssignmentLeaseValid(aiController)) continue;

		FEngageAssignmentSlotState& targetSlotState = InOutSlotState.FindOrAdd(previousAssignment.TargetActor);

		if (previousAssignment.CombatRole == ECombatRole::Engage)
		{
			if (targetSlotState.EngageCount >= MaxEngagersPerTarget) continue;

			InOutNextAssignments.Add(aiController, previousAssignment);
			++targetSlotState.EngageCount;
		}
		else if (previousAssignment.CombatRole == ECombatRole::Alert)
		{
			if (targetSlotState.AlertCount >= MaxAlertersPerTarget) continue;

			InOutNextAssignments.Add(aiController, previousAssignment);
			++targetSlotState.AlertCount;
		}
	}
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
	RequestContainer.Reset();
	LastRequestTimeContainer.Reset();
	AssignmentContainer.Reset();
}

// Debug

void UCWorldSubsystem_CombatEngage::PrintEngageContext(const ACAIController* InCAIController, const AActor* InActor, const int& InPriority, const int& InIndex, const float& InDistance, const ECombatRole& InCombatRole) const
{
	FLog::Log(TEXT("==== EngageAssignmentContext ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("AIController"), *GetNameSafe(InCAIController)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("TargetActor"), *GetNameSafe(InActor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Priority"), InPriority));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Index"), InIndex));
	FLog::Log(FString::Printf(TEXT("%-20s: %.3f"), TEXT("DistanceToTarget"), InDistance));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("CombatRole"), *UEnum::GetValueAsString(InCombatRole)));
	FLog::Log(TEXT("================================="));
}
