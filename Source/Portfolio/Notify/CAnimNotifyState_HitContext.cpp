#include "Notify/CAnimNotifyState_HitContext.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotifyState_HitContext::UCAnimNotifyState_HitContext()
{
}

FString UCAnimNotifyState_HitContext::GetNotifyName_Implementation() const
{
	return TEXT("Hit Context");
}

void UCAnimNotifyState_HitContext::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::PushHitContext);
}

void UCAnimNotifyState_HitContext::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::ClearHitContext);
}
