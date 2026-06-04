#include "Notify/CAnimNotifyState_ActionFeedback.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotifyState_ActionFeedback::UCAnimNotifyState_ActionFeedback()
{
}

FString UCAnimNotifyState_ActionFeedback::GetNotifyName_Implementation() const
{
	return TriggerKey.IsNone() ? TEXT("ActionFeedback(Window)") : FString::Printf(TEXT("ActionFeedback(Window: %s)"), *TriggerKey.ToString());
}

void UCAnimNotifyState_ActionFeedback::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionFeedbackWindowBegin(TriggerKey);
}

void UCAnimNotifyState_ActionFeedback::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionFeedbackWindowEnd(TriggerKey);
}
