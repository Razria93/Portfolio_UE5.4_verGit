#include "System/Combat/CWorldSubsystem_CombatEngage.h"
#include "ProjectGlobal.h"
#include "Core/Profiling/FPortfolioCsvProfiler.h"

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
	CSV_SCOPED_TIMING_STAT(PortfolioAI, CombatEngage_Tick);

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

	// Override Request
	RequestContainer.FindOrAdd(InEngageRequestContext.RequestController) = InEngageRequestContext;
}

// Assignment

void UCWorldSubsystem_CombatEngage::RebuildAssignments()
{
	CSV_SCOPED_TIMING_STAT(PortfolioAI, CombatEngage_RebuildAssignments);

	AssignmentContainer.Reset();

	// 1. Build RequestBucket from RequestContainer
	TMap<AActor*, TArray<FEngageRequestContext>> requestBucket;
	for (const TPair<ACAIController*, FEngageRequestContext>& pair : RequestContainer)
	{
		const ACAIController* requestor = pair.Key;
		const FEngageRequestContext& request = pair.Value;
		
		if (!IsValid(request.RequestController) || !IsValid(request.TargetActor)) continue;

		requestBucket.FindOrAdd(request.TargetActor).Add(request);
	}

	// 2. Build AssignmentContainer from RequestBucket
	for (auto& pair : requestBucket)
	{
		AActor* targetActor = pair.Key;
		TArray<FEngageRequestContext>& rqeusetContexts = pair.Value;

		rqeusetContexts.Sort([](const FEngageRequestContext& A, const FEngageRequestContext& B)
			{
				// SortCondition 1: Priority
				if (A.TargetPriority != B.TargetPriority)
					return A.TargetPriority < B.TargetPriority;

				// SortCondition 2: bWasEngaged
				// If A was engaged and B was not -> keep A ahead.
				// If A was not engaged and B was -> let B move ahead.
				if (A.bWasEngaged != B.bWasEngaged)
					return A.bWasEngaged; 

				// SortCondition 3: DistanceToTarget
				return A.DistanceToTarget < B.DistanceToTarget;
			});

		for (int32 i = 0; i < rqeusetContexts.Num(); ++i)
		{
			ACAIController* requestController = rqeusetContexts[i].RequestController;
			ECombatRole combatRole = i < MaxEngagersPerTarget ? ECombatRole::Engage : ECombatRole::Alert;

			FEngageAssignmentContext engageAssignmentContext;

			engageAssignmentContext.TargetActor = targetActor;
			engageAssignmentContext.CombatRole = combatRole;

			AssignmentContainer.Add(requestController, engageAssignmentContext);
			
			// PrintEngageContext(rqeusetContexts[i].RequestController, rqeusetContexts[i].TargetActor, rqeusetContexts[i].TargetPriority, i, rqeusetContexts[i].DistanceToTarget, combatRole);
		}
	}

	// 3. Clear RequestContainer
	RequestContainer.Reset();
}

// Runtime State

void UCWorldSubsystem_CombatEngage::ClearEngageRuntimeState()
{
	ElapsedTime = 0.f;
	RequestContainer.Reset();
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
