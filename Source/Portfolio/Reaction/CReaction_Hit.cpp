#include "Reaction/CReaction_Hit.h"
#include "ProjectGlobal.h"

void UCReaction_Hit::InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent)
{
	Super::InitializeReaction(InOwnerCharacter, InOwnerReactionComponent);
}

void UCReaction_Hit::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}
