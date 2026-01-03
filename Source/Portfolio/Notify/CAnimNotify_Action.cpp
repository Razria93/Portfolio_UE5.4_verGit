#include "Notify/CAnimNotify_Action.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"
#include "Weapon/CAction.h"

#include "Type/CWeaponStructure.h"

UCAnimNotify_Action::UCAnimNotify_Action()
{
}

FString UCAnimNotify_Action::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Action");
}

void UCAnimNotify_Action::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!actionComp) return;

	UObject* uobject = actionComp->GetAction(actionComp->GetCurActionType());
	if (!uobject) return;

	UCAction* action = Cast<UCAction>(uobject);

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: action->BeginPlayAction(); return;
	case EAnimNotifyFlow::End: action->EndPlayAction(); return;
	case EAnimNotifyFlow::Next: action->NextPlayAction(); return;
	}
}
