#include "Notify/CAnimNotify_ExitExecutionDownPresentation.h"

#include "Component/CReactionComponent.h"

UCAnimNotify_ExitExecutionDownPresentation::UCAnimNotify_ExitExecutionDownPresentation()
{
	TriggerReactionType = EReactionType::ExecutionRecovery;
}

FString UCAnimNotify_ExitExecutionDownPresentation::GetNotifyName_Implementation() const
{
	return TEXT("Exit Execution Down Presentation");
}

void UCAnimNotify_ExitExecutionDownPresentation::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	if (!CanProcessReactionNotify(reactionComp)) return;

	reactionComp->HandleReactionNotifyCommand(EReactionNotifyCommand::ExitExecutionDownPresentation);
}
