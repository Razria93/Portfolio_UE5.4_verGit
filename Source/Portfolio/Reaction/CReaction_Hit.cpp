#include "Reaction/CReaction_Hit.h"
#include "ProjectGlobal.h"

#include "Component/CStateComponent.h"

void UCReaction_Hit::InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent)
{
	Super::InitializeReaction(InOwnerCharacter, InOwnerReactionComponent);
}

void UCReaction_Hit::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

bool UCReaction_Hit::Begin(const FReactionData& reactionData)
{
	// TODO: Hit-only: Movement / VFX Triggers etc

	return Super::Begin(reactionData);
}
