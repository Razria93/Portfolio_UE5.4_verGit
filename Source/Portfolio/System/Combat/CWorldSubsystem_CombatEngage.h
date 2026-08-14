#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/Debug/FCombatEngageDebugTypes.h"
#include "Type/CEngageAssignmentTypes.h"
#include "CWorldSubsystem_CombatEngage.generated.h"

namespace CCombatEngageConstants
{
	constexpr float UnsetAssignmentWarmupStartTime = -1.f;
	constexpr float MissingAssignmentLeaseAge = -1.f;
	constexpr int32 InitialAssignmentRebuildId = 0;
	constexpr int32 FirstAssignmentRebuildId = 1;
}

UCLASS()
class PORTFOLIO_API UCWorldSubsystem_CombatEngage : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	FEngageAssignmentTuning AssignmentTuning;

private:
	float ElapsedTime = 0.f;
	float AssignmentWarmupStartTime = CCombatEngageConstants::UnsetAssignmentWarmupStartTime;
	bool bAssignmentWarmupCompleted = false;
	int32 AssignmentRebuildId = CCombatEngageConstants::InitialAssignmentRebuildId;

private:
	UPROPERTY()
	TMap<class ACAIController*, FEngageRequestContext> RequestContainer;

	UPROPERTY()
	TMap<class ACAIController*, float> LastRequestTimeContainer;

	UPROPERTY()
	TMap<class ACAIController*, FEngageAssignmentContext> AssignmentContainer;

public:
	// Lifecycle
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// Tick
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

public:
	// Query
	FEngageAssignmentContext GetAssignment(const class ACAIController* InCAIController) const;

public:
	// Request
	void SubmitRequest(const FEngageRequestContext& InEngageRequestContext);

public:
	// Assignment
	void RebuildAssignments();

private:
	// Assignment Build
	TMap<class ACAIController*, FEngageRequestContext> ConsumeRequestSnapshot();
	void BuildRequestBucket(const TMap<class ACAIController*, FEngageRequestContext>& InRequestSnapshot, TMap<class AActor*, TArray<FEngageRequestContext>>& OutRequestBucket) const;
	void SortRequestContexts(TArray<FEngageRequestContext>& InOutRequestContexts) const;
	bool IsCombatTargetRevisionCurrent(const class ACAIController* InAIController, const class AActor* InTargetActor, int32 InTargetRevision) const;

private:
	// Assignment Warmup
	void StartAssignmentWarmupIfNeeded();
	bool ShouldDelayAssignmentForWarmup() const;
	float GetAssignmentWarmupElapsedTime() const;

private:
	// Assignment Apply
	void PreserveExistingEngageAssignments(TMap<class ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const;
	void PromoteExistingAlertAssignments(const TMap<class AActor*, TArray<FEngageRequestContext>>& InRequestBucket, TMap<class ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const;
	void PreserveExistingAlertAssignments(TMap<class ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const;
	void ApplyFreshRequestAssignments(const TMap<class AActor*, TArray<FEngageRequestContext>>& InRequestBucket, TMap<class ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState, FEngageAssignmentRebuildDebugState& InOutDebugState) const;

private:
	// Assignment Lease
	bool IsAssignmentLeaseValid(const class ACAIController* InCAIController) const;
	bool TryReserveAssignmentSlot(const FEngageAssignmentContext& InAssignment, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState) const;

private:
	// Runtime State
	void ClearEngageRuntimeState();
};
