#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Type/CWorldSubsystemStructure.h"
#include "CWorldSubsystem_CombatEngage.generated.h"

UCLASS()
class PORTFOLIO_API UCWorldSubsystem_CombatEngage : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	float RebuildInterval = 0.1f;

	UPROPERTY()
	float AssignmentLeaseDuration = 0.5f;

private:
	float ElapsedTime = 0.f;
	float AssignmentWarmupStartTime = -1.f;
	bool bAssignmentWarmupCompleted = false;
	int32 AssignmentRebuildId = 0;

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
