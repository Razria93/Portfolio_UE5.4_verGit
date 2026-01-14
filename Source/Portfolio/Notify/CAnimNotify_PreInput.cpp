#include "Notify/CAnimNotify_PreInput.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"
#include "Action/CAction_ComboAttack.h"

UCAnimNotify_PreInput::UCAnimNotify_PreInput()
{
}

FString UCAnimNotify_PreInput::GetNotifyName_Implementation() const
{
	return MakeNotifyName("PreInput");
}

void UCAnimNotify_PreInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!actionComp) return;

	UCAction_ComboAttack* action_ComboAttack = Cast<UCAction_ComboAttack>(actionComp->GetAction(actionComp->GetCurActionType()));
	if (!action_ComboAttack) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: action_ComboAttack->OnEnablePreInput(); return;
	case EAnimNotifyFlow::End: action_ComboAttack->OffEnablePreInput(); return;
	}
}