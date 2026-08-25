#include "Notify/CAnimNotify_CommitExecution.h"

#include "Component/CActionComponent.h"

UCAnimNotify_CommitExecution::UCAnimNotify_CommitExecution()
{
}

FString UCAnimNotify_CommitExecution::GetNotifyName_Implementation() const
{
	return TEXT("Commit Execution");
}

void UCAnimNotify_CommitExecution::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::CommitExecution);
}
