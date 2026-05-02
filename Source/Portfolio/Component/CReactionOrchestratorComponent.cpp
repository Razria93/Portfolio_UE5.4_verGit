#include "Component/CReactionOrchestratorComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"

#include "Reaction/CReaction.h"

UCReactionOrchestratorComponent::UCReactionOrchestratorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCReactionOrchestratorComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	check(HealthComp_Cached);

	ReactionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCReactionComponent>();
	check(ReactionComp_Cached);
}

FReactionRequestResult UCReactionOrchestratorComponent::RequestReaction(const FDamageReactionRequest& InRequest)
{
	return FReactionRequestResult();
}

bool UCReactionOrchestratorComponent::CanAcceptReactionRequest(EReactionRequestRejectReason& OutRequestRejectReason) const
{
	return false;
}

bool UCReactionOrchestratorComponent::ResolveReactionContext(const FDamageReactionRequest& InRequest, FReactionContext& OutContext, EReactionType& OutReactionType, EReactionRequestRejectReason& OutRequestRejectReason)
{
	return false;
}

EReactionType UCReactionOrchestratorComponent::ResolveReactionType(const FTakeDamageResult& InTakeDamageResult) const
{
	return EReactionType();
}

FReactionExecutionPolicy UCReactionOrchestratorComponent::ResolveReactionPolicy(const FReactionContext& InContext, EReactionType InReactionType) const
{
	return FReactionExecutionPolicy();
}

FReactionOrchestrationQuery UCReactionOrchestratorComponent::BuildOrchestrationQuery(EReactionIntentSource InIntentSource, EReactionType InReactionType, const FReactionContext& InContext, const FReactionExecutionPolicy& InPolicy) const
{
	return FReactionOrchestrationQuery();
}

FReactionOrchestrationResult UCReactionOrchestratorComponent::OrchestrateQuery(const FReactionOrchestrationQuery& InQuery) const
{
	return FReactionOrchestrationResult();
}

bool UCReactionOrchestratorComponent::CanReplaceReaction(const FReactionContext& InCurrentContext, const FReactionContext& InIncomingContext, const FReactionExecutionPolicy& InIncomingPolicy, EReactionRequestRejectReason& OutRejectReason) const
{
	return false;
}

void UCReactionOrchestratorComponent::DispatchReactionDecision(const FReactionOrchestrationResult& InResult)
{
}

FReactionRequestResult UCReactionOrchestratorComponent::BuildRequestResult(const FReactionOrchestrationResult& InResult) const
{
	return FReactionRequestResult();
}
