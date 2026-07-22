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
	// 1. Lifecycle
	void InitializeRuntimeLODTierSnapshot();
	void ClearRuntimeLODTierSnapshot();

	// 2. Set
	void SetCurrentRuntimeLODTier(EAIRuntimeLODTier InTier);

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

};


