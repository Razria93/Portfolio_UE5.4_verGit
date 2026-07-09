#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Type/CWorldSubSystemStructure.h"
#include "CWorldSubsystem_CombatEngage.generated.h"

struct FEngageAssignmentSlotState
{
	int32 EngageCount = 0;
	int32 AlertCount = 0;
};

UCLASS()
class PORTFOLIO_API UCWorldSubsystem_CombatEngage : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	int32 MaxEngagersPerTarget = 2;

	UPROPERTY()
	int32 MaxAlertersPerTarget = 6;

	UPROPERTY()
	float RebuildInterval = 0.1f;

	UPROPERTY()
	float AssignmentLeaseDuration = 0.5f;

private:
	float ElapsedTime = 0.f;

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
	EAIUpdatePrecision GetAIUpdatePrecision(const class ACAIController* InCAIController) const;

public:
	// Request
	void SubmitRequest(const FEngageRequestContext & InEngageRequestContext);

public:
	// Assignment
	void RebuildAssignments();

private:
	// Assignment Build
	TMap<class ACAIController*, FEngageRequestContext> ConsumeRequestSnapshot();
	void BuildRequestBucket(const TMap<class ACAIController*, FEngageRequestContext>& InRequestSnapshot, TMap<class AActor*, TArray<FEngageRequestContext>>& OutRequestBucket) const;
	void SortRequestContexts(TArray<FEngageRequestContext>& InOutRequestContexts) const;

private:
	// Assignment Apply
	void ApplyFreshRequestAssignments(const TMap<class AActor*, TArray<FEngageRequestContext>>& InRequestBucket, TMap<class ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, TSet<class ACAIController*>& OutFreshRequestControllers, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState) const;
	void PreserveLeasedAssignments(TMap<class ACAIController*, FEngageAssignmentContext>& InOutNextAssignments, const TSet<class ACAIController*>& InFreshRequestControllers, TMap<class AActor*, struct FEngageAssignmentSlotState>& InOutSlotState) const;

private:
	// Assignment Lease
	bool IsAssignmentLeaseValid(const class ACAIController* InCAIController) const;

private:
	// Runtime State
	void ClearEngageRuntimeState();

private:
	// Debug
	void PrintEngageContext(const ACAIController* InCAIController, const AActor* InActor, const int& InPriority, const int& InIndex, const float& InDistance, const ECombatRole& InCombatRole) const;
};
