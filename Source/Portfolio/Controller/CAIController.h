#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Type/CAIStructure.h"
#include "CAIController.generated.h"

enum class EAIRuntimeLODTier : uint8;

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
	UPROPERTY(EditDefaultsOnly)
	class UBlackboardData* BlackboardAsset;

	UPROPERTY(EditDefaultsOnly)
	class UBehaviorTree* BehaviorTreeAsset;

protected:
	UPROPERTY(VisibleAnywhere)
	class UAIPerceptionComponent* AIPerceptionComp;

protected:
	UPROPERTY(Transient)
	class APawn* ControlledPawn_Cached;

protected:
	UPROPERTY(Transient)
	class UAISenseConfig_Sight* SightConfig;

private:
	UPROPERTY(Transient)
	bool bPerceptionDisabledForProfiling = false;

	EAIRuntimeLODTier CurrentRuntimeLODTier;

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
	// Runtime LOD Snapshot
	EAIRuntimeLODTier GetCurrentRuntimeLODTier() const;
	bool RefreshRuntimeLODTierFromBlackboard();

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
	// Runtime LOD Snapshot
	void InitializeRuntimeLODTierSnapshot();
	void ClearRuntimeLODTierSnapshot();

	// Mutation
	void SetCurrentRuntimeLODTier(EAIRuntimeLODTier InTier);

private:
	// Perception Profiling Gate
	void InitializePerceptionStateForProfiling();
	void ClearPerceptionStateForProfiling();

	// Condition
	bool ShouldDisableEnemyPerceptionForProfiling() const;

	// Mutation
	void DisableEnemyPerceptionForProfiling();
	void EnableEnemyPerceptionForProfiling();
	void SetPerceptionSenseEnabledForProfiling(bool bEnabled);

private:
	// Perception Candidate Audit
	void InitializePerceptionCandidateAudit();
	void ClearPerceptionCandidateAudit();

	// Condition
	bool ShouldAuditPerceptionCandidates() const;

	// Record
	void RecordRawPerceptionCandidate(class AActor* InActor);
	void RecordValidTargetProvider(class AActor* InActor);
	void RecordInvalidTargetProvider(class AActor* InActor);
	void RecordTargetDataMapSizeForAudit();

private:
	// Blackboard / Engage Latency Audit
	void InitializeBlackboardEngageLatencyAudit();
	void ClearBlackboardEngageLatencyAudit();

	// Condition
	bool ShouldAuditBlackboardEngageLatency() const;

};


