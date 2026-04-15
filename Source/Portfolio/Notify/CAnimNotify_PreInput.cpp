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

	UObject* uobject = actionComp->GetCurAction();
	if (!uobject) return;

	UCAction_ComboAttack* action_ComboAttack = Cast<UCAction_ComboAttack>(uobject);
	if (!action_ComboAttack) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin:
	{
		// FLog::Log(TEXT("[AnimNotify|PreInput] Begin"));
		action_ComboAttack->OnEnablePreInput();
		break;
	}
	case EAnimNotifyFlow::End:
	{
		// FLog::Log(TEXT("[AnimNotify|PreInput] End"));
		action_ComboAttack->OffEnablePreInput();
		break;
	}
	}
}