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
	class UCHealthComponent* HealthComp_Cached = nullptr;

	UPROPERTY(Transient)
	class UCReactionComponent* ReactionComp_Cached = nullptr;

protected:
	virtual void BeginPlay() override;

public:
	FReactionRequestResult RequestReaction(const FDamageReactionRequest& InRequest);

private:
	bool CanAcceptReactionRequest(EReactionRequestRejectReason& OutRejectReason) const;

private:
	bool ResolveReactionContext(const FDamageReactionRequest& InRequest, FReactionContext& OutContext, EReactionType& OutReactionType, EReactionRequestRejectReason& OutRejectReason);
	EReactionType ResolveReactionType(const FTakeDamageResult& InResult) const;
	FReactionExecutionPolicy ResolveReactionPolicy(const FReactionContext& InContext, EReactionType InType) const;

private:
	FReactionOrchestrationQuery BuildOrchestrationQuery(EReactionIntentSource InIntentSource, EReactionType InType, const FReactionContext& InContext, const FReactionExecutionPolicy& InPolicy) const;
	FReactionOrchestrationResult OrchestrateQuery(const FReactionOrchestrationQuery& InQuery) const;

private:
	bool CanReplaceReaction(const FReactionContext& InCurrentContext, const FReactionContext& InIncomingContext, const FReactionExecutionPolicy& InIncomingPolicy, EReactionRequestRejectReason& OutRejectReason) const;
	
private:
	void DispatchReactionDecision(const FReactionOrchestrationResult& InResult);

private:
	FReactionRequestResult BuildRequestResult(const FReactionOrchestrationResult& InResult) const;
};
