#include "Notify/CAnimNotify_Action.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"
#include "Action/CAction.h"

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

	UCAction* curAction = actionComp->GetCurAction();
	if (!curAction) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: curAction->BeginPlayAction(); return;
	case EAnimNotifyFlow::End: curAction->EndPlayAction(); return;
	case EAnimNotifyFlow::Next: curAction->NextPlayAction(); return;
	}
}
