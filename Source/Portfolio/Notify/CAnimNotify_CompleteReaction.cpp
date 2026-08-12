#include "Notify/CAnimNotify_CompleteReaction.h"

#include "ProjectGlobal.h"

#include "Component/CReactionComponent.h"

UCAnimNotify_CompleteReaction::UCAnimNotify_CompleteReaction()
{
}

FString UCAnimNotify_CompleteReaction::GetNotifyName_Implementation() const
{
	return TEXT("Complete Reaction");
}

void UCAnimNotify_CompleteReaction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	if (!CanProcessReactionNotify(reactionComp)) return;

	reactionComp->HandleReactionNotifyCommand(EReactionNotifyCommand::Complete);
}
