#include "Notify/CAnimNotifyState_ChainWindow.h"
#include "ProjectGlobal.h"

#include "Component/CActionComponent.h"

UCAnimNotifyState_ChainWindow::UCAnimNotifyState_ChainWindow()
{
	TriggerActionType = EActionType::ComboAttack;
}

FString UCAnimNotifyState_ChainWindow::GetNotifyName_Implementation() const
{
	return TEXT("Chain Window");
}

void UCAnimNotifyState_ChainWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::OpenChainWindow);
}

void UCAnimNotifyState_ChainWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UCActionComponent* actionComp = GetActionComponent(MeshComp);
	if (!CanProcessActionNotify(actionComp)) return;

	actionComp->HandleActionNotifyCommand(EActionNotifyCommand::CloseChainWindow);
}