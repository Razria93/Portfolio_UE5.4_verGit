#include "Notify/CAnimNotify_ChainWindow.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction_ComboAttack.h"

UCAnimNotify_ChainWindow::UCAnimNotify_ChainWindow()
{
}

FString UCAnimNotify_ChainWindow::GetNotifyName_Implementation() const
{
	switch (NotifyType)
	{
	case EChainWindowNotifyType::Opened:
		return TEXT("Chain Window(Opened)");

	case EChainWindowNotifyType::Closed:
		return TEXT("Chain Window(Closed)");

	default:
		return TEXT("Chain Window");
	}
}

void UCAnimNotify_ChainWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!actionComp) return;

	UCAction* currentAction = actionComp->GetCurrentAction();
	if (!currentAction) return;

	UCAction_ComboAttack* currentAction_ComboAttack = Cast<UCAction_ComboAttack>(currentAction);
	if (!currentAction_ComboAttack) return;

	if (!CanProcessActionNotify(currentAction_ComboAttack)) return;

	switch (NotifyType)
	{
	case EChainWindowNotifyType::Opened:
	{
		currentAction_ComboAttack->OpenChainWindow();
		break;
	}
	case EChainWindowNotifyType::Closed:
	{
		currentAction_ComboAttack->CloseChainWindow();
		break;
	}
	}
}