#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "CReactionOrchestratorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionOrchestratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCReactionOrchestratorComponent();

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

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

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Request Entry
	FReactionRequestResult RequestDamageReaction(const FDamageReactionRequest& InIncomingRequest);
	FReactionRequestResult RequestBalanceLifecycleReaction(const FBalanceLifecycleReactionRequest& InIncomingRequest);

private:
	// Request Validation
	bool CanAcceptReactionRequest(EReactionRequestRejectReason& OutRejectReason) const;

private:
	// Candidate Resolve - Damage
	bool ResolveDamageReactionCandidate(const FDamageReactionRequest& InIncomingRequest, FReactionCandidate& OutIncomingCandidate, EReactionRequestRejectReason& OutRejectReason) const;
	EReactionType ResolveDamageReactionType(const FDamageReactionRequest& InIncomingRequest) const;

	// Candidate Resolve - Balance Lifecycle
	bool ResolveBalanceLifecycleReactionCandidate(const FBalanceLifecycleReactionRequest& InIncomingRequest, FReactionCandidate& OutIncomingCandidate, EReactionRequestRejectReason& OutRejectReason) const;

private:
	// Orchestration Pipeline
	FReactionRequestResult ProcessReactionCandidate(const FReactionCandidate& InIncomingCandidate);

private:
	// Execution Context Resolve
	bool ResolveReactionContext(const FReactionCandidate& InIncomingCandidate, FReactionExecutionContext& OutIncomingContext, EReactionRequestRejectReason& OutRejectReason) const;
	
	bool ResolveReactionData(const FReactionDataKey& InIncomingDataKey, FReactionData& OutIncomingData) const;
	class UCReaction* ResolveReactionExecutor(const FReactionData& InIncomingData) const;

private:
	// Decision Query Build
	FExecutionDecisionQuery BuildDecisionQuery(const FReactionExecutionContext& InIncomingContext) const;

	FExecutionSnapshot BuildSnapshot() const;
	FExecutionParticipant BuildIncomingReactionParticipant(const FReactionExecutionContext& InIncomingContext) const;
	FExecutionParticipant BuildActiveExecutionParticipant() const;

private:
	// Decision Build
	FExecutionDecisionResult BuildDecisionResult(const FExecutionDecisionQuery& InQuery, EReactionRequestRejectReason& OutRejectReason) const;

private:
	FReactionExecutionResult BuildReactionExecutionResult(const FReactionExecutionContext& InContext, const FExecutionDecisionResult& InDecisionResult, EReactionRequestRejectReason InRejectReason) const;

private:
	// Decision Refinement
	void ResolveExecutionApplyMode(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const;
	void ResolveInterventionDirective(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const;
	void ResolveObservableOverlayGate(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const;

private:
	// Intervention Build
	bool BuildInterventionQuery(const FExecutionDecisionQuery& InQuery, EExecutionStopReason InStopReason, FExecutionInterventionQuery& OutQuery) const;
	bool BuildInterventionDirective(const FExecutionInterventionQuery& InQuery, EExecutionStopSource InStopSource, EExecutionAfterStopAction InAfterStopAction, FExecutionInterventionDirective& OutDirective) const;

private:
	// Decision Dispatch
	FReactionRequestResult DispatchReactionDecision(const FReactionExecutionResult& InResult);

private:
	// Result Build
	EReactionRequestResultType ConvertDecisionToResultType(const FReactionExecutionResult& InResult) const;
	FReactionRequestResult BuildReactionRequestResult(EReactionRequestResultType InResultType, EReactionRequestRejectReason InRejectReason = EReactionRequestRejectReason::None) const;
};
