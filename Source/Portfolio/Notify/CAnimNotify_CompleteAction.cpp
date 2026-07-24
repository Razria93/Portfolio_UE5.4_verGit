#include "Notify/CAnimNotify_CompleteAction.h"

#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotify_CompleteAction::UCAnimNotify_CompleteAction()
{
}

FString UCAnimNotify_CompleteAction::GetNotifyName_Implementation() const
{
	return TEXT("Complete Action");
}

void UCAnimNotify_CompleteAction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::Complete);
}
