#include "Notify/CAnimNotify_AdvanceCombo.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotify_AdvanceCombo::UCAnimNotify_AdvanceCombo()
{
	TriggerActionType = EActionType::ComboAttack;
}

FString UCAnimNotify_AdvanceCombo::GetNotifyName_Implementation() const
{
	return TEXT("Advance Combo");
}

void UCAnimNotify_AdvanceCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::ConsumeChain);
}