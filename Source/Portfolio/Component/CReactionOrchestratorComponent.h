#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CReactionOrchestrationStructure.h"
#include "CReactionOrchestratorComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCReactionOrchestratorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCReactionOrchestratorComponent();

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	class UCStateComponent* StateComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCHealthComponent* HealthComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCActionComponent* ActionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCObservableOverlayComponent* ObservableOverlayComp_Cached = nullptr;

protected:
	void BeginPlay() override;

public:
	FReactionRequestResult RequestDamageReaction(const FDamageReactionRequest& InIncomingRequest);

private:
	bool CanAcceptReactionRequest(EReactionRequestRejectReason& OutRejectReason) const;

private:
	bool ResolveDamageReactionCandidate(const FDamageReactionRequest& InIncomingRequest, FReactionCandidate& OutIncomingCandidate, EReactionRequestRejectReason& OutRejectReason) const;
	
	// Inner API
	EReactionType ResolveDamageReactionType(const FDamageReactionRequest& InIncomingRequest) const;

private:
	FReactionRequestResult ExecuteReactionCandidate(const FReactionCandidate& InIncomingCandidate);

private:
	bool ResolveReactionContext(const FReactionCandidate& InIncomingCandidate, FReactionExecutionContext& OutIncomingContext, EReactionRequestRejectReason& OutRejectReason) const;

	// Inner API
	bool ResolveReactionData(const FReactionDataKey& InIncomingDataKey, FReactionData& OutIncomingData) const;
	class UCReaction* ResolveReactionExecutor(const FReactionData& InIncomingData) const;

private:
	FExecutionDecisionQuery BuildDecisionQuery(const FReactionExecutionContext& InIncomingContext) const;

	// Inner API
	FExecutionSnapshot BuildSnapshot() const;
	FExecutionParticipant BuildIncomingReactionParticipant(const FReactionExecutionContext& InIncomingContext) const;
	FExecutionParticipant BuildActiveExecutionParticipant() const;

private:
	FExecutionDecisionResult BuildDecisionResult(const FExecutionDecisionQuery& InQuery, EReactionRequestRejectReason& OutRejectReason) const;

private:
	FReactionExecutionResult BuildReactionExecutionResult(const FReactionExecutionContext& InContext, const FExecutionDecisionResult& InDecisionResult, EReactionRequestRejectReason InRejectReason) const;

private:
	void ResolveExecutionApplyMode(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const;
	void ResolveObservableOverlayGate(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const;

	// Inner API
	void ResolveInterventionDirective(const FExecutionDecisionQuery& InQuery, FReactionExecutionResult& InOutResult) const;
	
	bool BuildInterventionQuery(const FExecutionDecisionQuery& InQuery, EExecutionStopReason InStopReason, FExecutionInterventionQuery& OutQuery) const;
	bool BuildInterventionDirective(const FExecutionInterventionQuery& InQuery, EExecutionStopSource InStopSource, EExecutionAfterStopAction InAfterStopAction, FExecutionInterventionDirective& OutDirective) const;

private:
	FReactionRequestResult DispatchReactionDecision(const FReactionExecutionResult& InResult);

private:
	EReactionRequestResultType ConvertDecisionToResultType(const FReactionExecutionResult& InResult) const;
	FReactionRequestResult BuildReactionRequestResult(EReactionRequestResultType InResultType, EReactionRequestRejectReason InRejectReason = EReactionRequestRejectReason::None) const;

private:
	void PrintReactionRequestResult(const FReactionRequestResult& InResult) const;
};
