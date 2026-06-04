#include "Notify/CAnimNotify_ActionFeedback.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotify_ActionFeedback::UCAnimNotify_ActionFeedback()
{
}

FString UCAnimNotify_ActionFeedback::GetNotifyName_Implementation() const
{
	return TriggerKey.IsNone() ? TEXT("ActionFeedback(Point)") : FString::Printf(TEXT("ActionFeedback(Point: %s)"), *TriggerKey.ToString());
}

void UCAnimNotify_ActionFeedback::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionFeedback(TriggerKey);
}
