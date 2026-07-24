#include "Notify/CAnimNotify_Equip.h"

#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotify_Equip::UCAnimNotify_Equip()
{
	TriggerActionType = EActionType::Equip;
}

FString UCAnimNotify_Equip::GetNotifyName_Implementation() const
{
	return TEXT("Equip");
}

void UCAnimNotify_Equip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::Equip);
}
