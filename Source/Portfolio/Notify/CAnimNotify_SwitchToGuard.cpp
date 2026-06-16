#include "Notify/CAnimNotify_SwitchToGuard.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotify_SwitchToGuard::UCAnimNotify_SwitchToGuard()
{
	TriggerActionType = EActionType::Guard;
	TriggerActionIndex = 1;
}

FString UCAnimNotify_SwitchToGuard::GetNotifyName_Implementation() const
{
	return TEXT("SwitchToGuard");
}

void UCAnimNotify_SwitchToGuard::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->NotifySwitchToGuard();
}
