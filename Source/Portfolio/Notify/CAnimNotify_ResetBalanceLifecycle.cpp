#include "Notify/CAnimNotify_ResetBalanceLifecycle.h"

#include "Component/CReactionComponent.h"

UCAnimNotify_ResetBalanceLifecycle::UCAnimNotify_ResetBalanceLifecycle()
{
	TriggerReactionType = EReactionType::All;
}

FString UCAnimNotify_ResetBalanceLifecycle::GetNotifyName_Implementation() const
{
	return TEXT("Reset Balance Lifecycle");
}

void UCAnimNotify_ResetBalanceLifecycle::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	if (!CanProcessReactionNotify(reactionComp)) return;

	reactionComp->HandleReactionNotifyCommand(EReactionNotifyCommand::ResetBalance);
}
