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
	
	UCAction* action = actionComp->GetAction(actionComp->GetCurActionType());
	if (!action) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: action->BeginPlayAction(); return;
	case EAnimNotifyFlow::End: action->EndPlayAction(); return;
	case EAnimNotifyFlow::Next: action->NextPlayAction(); return;
	}
}
