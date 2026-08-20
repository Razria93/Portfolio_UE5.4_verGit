#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Core/Debug/FAIPerceptionDebugTypes.h"
#include "Type/CAITypes.h"
#include "Type/CAIPerceptionSetupTypes.h"
#include "CAIController.generated.h"

enum class EAIRuntimeLODTier : uint8;

UCLASS()
class PORTFOLIO_API ACAIController : public AAIController
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AI|Perception")
	FAIControllerPerceptionSetup PerceptionSetup;

protected:
	UPROPERTY(Transient)
	TMap<AActor*, FPerceptionTargetContext> PerceptionTargetContextMap;

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
	bool ConfigureSightConfig();

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

	// Blackboard Runtime Value
	bool InitializeBlackboardValues();
	void InitializeCustomBlackboardValues(class UBlackboardComponent* InBlackboardComp, const class APawn* InOwnerPawn, TSet<FName>& InOutPendingKeys) const;

	void ClearBlackboardValues();

	// Behavior Tree Runtime
	bool StartBehaviorTreeRuntime();
	void StopBehaviorTreeRuntime();

private:
	// Perception Event Callback
	UFUNCTION()
	void OnTargetPerceptionUpdated(class AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void OnTargetPerceptionForgotten(class AActor* Actor);

public:
	// Query
	EPerceptionBuildResult BuildPerceptionContext(FPerceptionTargetContext& OutPerceptionTargetContext);
	void RefreshParticipationEvidenceFromPerception();

public:
	// Runtime LOD Snapshot
	EAIRuntimeLODTier GetCurrentRuntimeLODTier() const;
	bool RefreshRuntimeLODTierFromBlackboard();

public:
	// Perception Audit Event Sink
	void RecordPerceptionContextBuiltForAudit(class AActor* InTargetActor);
	void RecordBlackboardTargetSetForAudit(class AActor* InTargetActor);
	void RecordEngageRequestSubmittedForAudit(class AActor* InTargetActor);
	void RecordEngageAssignmentResolvedForAudit(class AActor* InTargetActor);

private:
	// Target Data
	void UpdatePerceptionTargetContextMap();
	void ClearPerceptionTargetContextMap();
	EPerceptionBuildResult SelectTopPriority(FPerceptionTargetContext& OutPerceptionTargetContext) const;

private:
	// Runtime LOD Snapshot
	void InitializeRuntimeLODTierSnapshot();
	void ClearRuntimeLODTierSnapshot();
	void SetCurrentRuntimeLODTier(EAIRuntimeLODTier InTier);

private:
	// Perception Profiling Gate
	void InitializePerceptionStateForProfiling();
	void ClearPerceptionStateForProfiling();
	bool ShouldDisableEnemyPerceptionForProfiling() const;
	void DisableEnemyPerceptionForProfiling();
	void EnableEnemyPerceptionForProfiling();
	void SetPerceptionSenseEnabledForProfiling(bool bEnabled);

private:
	// Perception Candidate Audit
	void InitializePerceptionCandidateAudit();
	void ClearPerceptionCandidateAudit();
	bool ShouldAuditPerceptionCandidates() const;
	void RecordRawPerceptionCandidate(class AActor* InActor);
	void RecordValidTargetProvider(class AActor* InActor);
	void RecordInvalidTargetProvider(class AActor* InActor);
	void RecordPerceptionTargetContextMapSizeForAudit();

private:
	// Blackboard / Engage Latency Audit
	void InitializeBlackboardEngageLatencyAudit();
	void ClearBlackboardEngageLatencyAudit();
	bool ShouldAuditBlackboardEngageLatency() const;

};
