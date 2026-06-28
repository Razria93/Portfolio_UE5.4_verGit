#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CActionOrchestrationStructure.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "CActionOrchestratorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCActionOrchestratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCActionOrchestratorComponent();

public:
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	class UCMovementComponent* MovementComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCWeaponComponent* WeaponComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCObservableOverlayComponent* ObservableOverlayComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Injected = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Injected = nullptr;

private:
	UPROPERTY(Transient)
	TArray<FDeferredActionCandidate> DeferredActionCandidates;

protected:
	// Lifecycle
	void BeginPlay() override;

private:
	// Component Reference
	bool ValidateRequiredComponentReferences() const;

public:
	// Request Entry
	FActionRequestResult RequestMovementAction(const FMovementActionRequest& InIncomingRequest);
	FActionRequestResult RequestEquipmentAction(const FEquipmentActionRequest& InIncomingRequest);
	FActionRequestResult RequestCombatAction(const FCombatActionRequest& InIncomingRequest);

public:
	// Deferred Entry
	FActionRequestResult ConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey);

public:
	// Deferred Management
	void ClearAllDeferredActions();
	void ClearDeferredActions(EDeferredActionConsumeKey InConsumeKey);
	void ClearDeferredActions(EDeferredActionConsumeKey InConsumeKey, const FActionDataKey& InActionDataKey);

private:
	// Request Validation
	bool CanAcceptActionRequest(EActionRequestRejectReason& OutRejectReason) const;

private:
	// Candidate Resolve
	bool ResolveEquipmentActionCandidate(const FEquipmentActionRequest& InIncomingRequest, FActionCandidate& OutIncomingCandidate, EActionRequestRejectReason& OutRejectReason) const;
	bool ResolveCombatActionCandidate(const FCombatActionRequest& InIncomingRequest, FActionCandidate& OutIncomingCandidate, EActionRequestRejectReason& OutRejectReason) const;

private:
	// Request Side Effects
	void ApplyCombatActionInputSideEffects(const FCombatActionRequest& InIncomingRequest) const;

private:
	// Orchestration Pipeline
	FActionRequestResult ProcessActionCandidate(const FActionCandidate& InIncomingCandidate);

private:
	// Execution Context Resolve
	bool ResolveActionContext(const FActionCandidate& InIncomingCandidate, FActionExecutionContext& OutIncomingContext, EActionRequestRejectReason& OutRejectReason) const;

	bool ResolveActionData(const FActionDataKey& InIncomingDataKey, FActionData& OutIncomingData) const;
	class UCAction* ResolveActionExecutor(const FActionData& InIncomingData) const;

private:
	// Decision Query Build
	FExecutionDecisionQuery BuildDecisionQuery(const FActionExecutionContext& InIncomingContext) const;

	FExecutionSnapshot BuildSnapshot() const;
	FExecutionParticipant BuildIncomingActionParticipant(const FActionExecutionContext& InIncomingContext) const;
	FExecutionParticipant BuildActiveExecutionParticipant() const;

private:
	// Deferred Resolve
	bool TryResolveDeferredConsumeKey(const FActionCandidate& InIncomingCandidate, const FExecutionDecisionQuery& InQuery, EDeferredActionConsumeKey& OutConsumeKey) const;
	FActionRequestResult DeferActionCandidate(const FActionCandidate& InIncomingCandidate, EDeferredActionConsumeKey InConsumeKey);

private:
	// Decision Build
	FExecutionDecisionResult BuildDecisionResult(const FExecutionDecisionQuery& InQuery, EActionRequestRejectReason& OutRejectReason) const;

private:
	FActionExecutionResult BuildActionExecutionResult(const FActionExecutionContext& InContext, const FExecutionDecisionResult& InDecisionResult, EActionRequestRejectReason InRejectReason) const;

private:
	// Decision Refinement
	void ResolveExecutionApplyMode(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const;
	void ResolveInterventionDirective(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const;
	void ResolveObservableOverlayGate(const FExecutionDecisionQuery& InQuery, FActionExecutionResult& InOutResult) const;

private:
	// Intervention Build
	bool BuildInterventionQuery(const FExecutionDecisionQuery& InQuery, EExecutionStopReason InStopReason, FExecutionInterventionQuery& OutQuery) const;
	bool BuildInterventionDirective(const FExecutionInterventionQuery& InQuery, EExecutionStopSource InStopSource, EExecutionAfterStopAction InAfterStopAction, FExecutionInterventionDirective& OutDirective) const;

private:
	// Decision Dispatch
	FActionRequestResult DispatchActionDecision(const FActionExecutionResult& InResult);

private:
	// Result Build
	EActionRequestResultType ConvertDecisionToResultType(const FActionExecutionResult& InResult) const;
	FActionRequestResult BuildActionRequestResult(EActionRequestResultType InResultType, EActionRequestRejectReason InRejectReason = EActionRequestRejectReason::None) const;
};
