#include "Notify/CAnimNotifyState_ReactionFeedback.h"
#include "ProjectGlobal.h"

#include "Component/CReactionComponent.h"

UCAnimNotifyState_ReactionFeedback::UCAnimNotifyState_ReactionFeedback()
{
}

FString UCAnimNotifyState_ReactionFeedback::GetNotifyName_Implementation() const
{
	return TriggerKey.IsNone() ? TEXT("ReactionFeedback(Window)") : FString::Printf(TEXT("ReactionFeedback(Window: %s)"), *TriggerKey.ToString());
}

void UCAnimNotifyState_ReactionFeedback::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	if (!CanProcessReactionNotify(reactionComp)) return;

	reactionComp->HandleReactionFeedbackWindowBegin(TriggerKey);
}

void UCAnimNotifyState_ReactionFeedback::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	if (!CanProcessReactionNotify(reactionComp)) return;

	reactionComp->HandleReactionFeedbackWindowEnd(TriggerKey);
}
