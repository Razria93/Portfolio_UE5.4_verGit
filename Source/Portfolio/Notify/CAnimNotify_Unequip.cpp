#include "Notify/CAnimNotify_Unequip.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotify_Unequip::UCAnimNotify_Unequip()
{
	TriggerActionType = EActionType::Unequip;
}

FString UCAnimNotify_Unequip::GetNotifyName_Implementation() const
{
	return TEXT("Unequip");
}

void UCAnimNotify_Unequip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::Unequip);
}