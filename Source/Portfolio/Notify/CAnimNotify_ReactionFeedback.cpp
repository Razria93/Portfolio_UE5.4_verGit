#include "Notify/CAnimNotify_ReactionFeedback.h"
#include "ProjectGlobal.h"

#include "Component/CReactionComponent.h"

UCAnimNotify_ReactionFeedback::UCAnimNotify_ReactionFeedback()
{
}

FString UCAnimNotify_ReactionFeedback::GetNotifyName_Implementation() const
{
	return TriggerKey.IsNone() ? TEXT("ReactionFeedback(Point)") : FString::Printf(TEXT("ReactionFeedback(Point: %s)"), *TriggerKey.ToString());
}

void UCAnimNotify_ReactionFeedback::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	if (!CanProcessReactionNotify(reactionComp)) return;

	reactionComp->HandleReactionFeedback(TriggerKey);
}
