#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Type/CAIStructure.h"
#include "CAIController.generated.h"

struct FPerceptionCandidateAuditState
{
	bool bEnabled = false;

	float RuntimeStartTime = 0.f;
	uint64 RuntimeStartFrame = 0;

	float FirstRawPerceptionTime = -1.f;
	uint64 FirstRawPerceptionFrame = 0;

	float FirstValidTargetTime = -1.f;
	uint64 FirstValidTargetFrame = 0;

	int32 RawPerceptionEventCount = 0;
	int32 MaxTargetDataMapSize = 0;

	TSet<TWeakObjectPtr<class AActor>> RawPerceptionActors;
	TSet<TWeakObjectPtr<class AActor>> ValidTargetProviderActors;
	TSet<TWeakObjectPtr<class AActor>> InvalidTargetProviderActors;

	void Reset()
	{
		bEnabled = false;

		RuntimeStartTime = 0.f;
		RuntimeStartFrame = 0;

		FirstRawPerceptionTime = -1.f;
		FirstRawPerceptionFrame = 0;

		FirstValidTargetTime = -1.f;
		FirstValidTargetFrame = 0;

		RawPerceptionEventCount = 0;
		MaxTargetDataMapSize = 0;

		RawPerceptionActors.Reset();
		ValidTargetProviderActors.Reset();
		InvalidTargetProviderActors.Reset();
	}
};

struct FBlackboardEngageLatencyAuditState
{
	bool bEnabled = false;

	float RuntimeStartTime = 0.f;
	uint64 RuntimeStartFrame = 0;

	float FirstPerceptionContextTime = -1.f;
	uint64 FirstPerceptionContextFrame = 0;

	float FirstBlackboardTargetTime = -1.f;
	uint64 FirstBlackboardTargetFrame = 0;

	float FirstEngageRequestTime = -1.f;
	uint64 FirstEngageRequestFrame = 0;

	float FirstEngageAssignmentTime = -1.f;
	uint64 FirstEngageAssignmentFrame = 0;

	TWeakObjectPtr<class AActor> FirstPerceptionTargetActor;
	TWeakObjectPtr<class AActor> FirstBlackboardTargetActor;
	TWeakObjectPtr<class AActor> FirstEngageRequestTargetActor;
	TWeakObjectPtr<class AActor> FirstEngageAssignmentTargetActor;

	void Reset()
	{
		bEnabled = false;

		RuntimeStartTime = 0.f;
		RuntimeStartFrame = 0;

		FirstPerceptionContextTime = -1.f;
		FirstPerceptionContextFrame = 0;

		FirstBlackboardTargetTime = -1.f;
		FirstBlackboardTargetFrame = 0;

		FirstEngageRequestTime = -1.f;
		FirstEngageRequestFrame = 0;

		FirstEngageAssignmentTime = -1.f;
		FirstEngageAssignmentFrame = 0;

		FirstPerceptionTargetActor.Reset();
		FirstBlackboardTargetActor.Reset();
		FirstEngageRequestTargetActor.Reset();
		FirstEngageAssignmentTargetActor.Reset();
	}
};

UCLASS()
class PORTFOLIO_API ACAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	float TargetMemoryTimeout = 3.0f;

protected:
	UPROPERTY(Transient)
	TMap<AActor*, FTargetData> TargetDataMap;

protected:
	/* --- Asset --- */
	UPROPERTY(EditDefaultsOnly)
	class UBlackboardData* BlackboardAsset;

	UPROPERTY(EditDefaultsOnly)
	class UBehaviorTree* BehaviorTreeAsset;

protected:
	/* --- Component --- */
	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerceptionComp;

protected:
	/* --- Cached --- */
	UPROPERTY(Transient)
	class APawn* ControlledPawn_Cached;

protected:
	/* --- Config --- */
	UPROPERTY(Transient)
	class UAISenseConfig_Sight* SightConfig;

private:
	// Profiling State
	UPROPERTY(Transient)
	bool bPerceptionDisabledForProfiling = false;

	FPerceptionCandidateAuditState PerceptionCandidateAuditState;
	FBlackboardEngageLatencyAuditState BlackboardEngageLatencyAuditState;

public:
	ACAIController();

protected:
	// Lifecycle
	void BeginPlay() override;
	void OnPossess(class APawn* InPawn) override;
	void OnUnPossess() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Team
	ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	// Config Setup
	bool InitializeSightConfig();

private:
	// Runtime Lifecycle
	bool InitializeControllerRuntime(class APawn* InPawn);
	void UninitializeControllerRuntime();

private:
	// Possession Runtime
	bool SetPossessionRuntimeState(class APawn* InPawn);
	void ResetPossessionRuntimeState();

	// Perception Binding
	bool BindPerceptionEvents();
	void UnbindPerceptionEvents();

	// Blackboard Setup
	bool SetupBlackboardComponent();

	// Blackboard Value
	bool InitializeBlackboardValues();
	void InitializeCustomBlackboardValues(class UBlackboardComponent* InBlackboardComp, const class APawn* InOwnerPawn, TSet<FName>& InOutPendingKeys) const;

	void ClearBlackboardValues();

	// Behavior Tree Runtime
	bool StartBehaviorTreeRuntime();
	void StopBehaviorTreeRuntime();

private:
	// Perception Event Callback
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<class AActor*>& InUpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionUpdated(class AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnTargetPerceptionForgotten(class AActor* Actor);

public:
	// Query
	EPerceptionBuildResult BuildPerceptionContext(FTargetData& OutTargetData);

public:
	// Profiling Event Sink
	void RecordPerceptionContextBuiltForAudit(class AActor* InTargetActor);
	void RecordBlackboardTargetSetForAudit(class AActor* InTargetActor);
	void RecordEngageRequestSubmittedForAudit(class AActor* InTargetActor);
	void RecordEngageAssignmentResolvedForAudit(class AActor* InTargetActor);

private:
	// Target Data
	void UpdateTargetDataMap();
	void ClearTargetDataMap();
	EPerceptionBuildResult SelectTopPriority(FTargetData& OutTargetData);

private:
	// Perception Profiling Gate
	// 1. Lifecycle
	void InitializePerceptionStateForProfiling();
	void ClearPerceptionStateForProfiling();

	// 2. Condition
	bool ShouldDisableEnemyPerceptionForProfiling() const;

	// 3. Set
	void DisableEnemyPerceptionForProfiling();
	void EnableEnemyPerceptionForProfiling();
	void SetPerceptionSenseEnabledForProfiling(bool bEnabled);

private:
	// Perception Candidate Audit
	// 1. Lifecycle
	void InitializePerceptionCandidateAudit();
	void ClearPerceptionCandidateAudit();

	// 2. Condition
	bool ShouldAuditPerceptionCandidates() const;

	// 3. Record
	void RecordRawPerceptionCandidate(class AActor* InActor);
	void RecordValidTargetProvider(class AActor* InActor);
	void RecordInvalidTargetProvider(class AActor* InActor);
	void RecordTargetDataMapSizeForAudit();

private:
	// Blackboard / Engage Latency Audit
	// 1. Lifecycle
	void InitializeBlackboardEngageLatencyAudit();
	void ClearBlackboardEngageLatencyAudit();

	// 2. Condition
	bool ShouldAuditBlackboardEngageLatency() const;

private:
	// Debug
	void PrintPerceptionUpdatedSummary(const TArray<class AActor*>& UpdatedActors) const;
	void PrintTargetPerceptionUpdatedSummary(class AActor* Actor, const FAIStimulus& Stimulus) const;
	void PrintTargetPerceptionForgotten(AActor* Actor) const;
	void PrintAllTargetData() const;
	void PrintTargetData(const FTargetData& InData) const;

	// Profiling Debug
	void PrintPerceptionCandidateAuditSummary() const;
	void PrintBlackboardEngageLatencyAuditSummary() const;
};


