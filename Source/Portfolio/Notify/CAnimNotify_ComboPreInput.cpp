#include "Notify/CAnimNotify_ComboPreInput.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction_ComboAttack.h"

UCAnimNotify_ComboPreInput::UCAnimNotify_ComboPreInput()
{
}

FString UCAnimNotify_ComboPreInput::GetNotifyName_Implementation() const
{
	switch (NotifyType)
	{
	case EPreInputNotifyType::Enabled:
		return TEXT("Combo PreInput(Enabled)");

	case EPreInputNotifyType::Disabled:
		return TEXT("Combo PreInput(Disabled)");

	default:
		return TEXT("Combo PreInput");
	}
}

void UCAnimNotify_ComboPreInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!actionComp) return;

	UCAction* currentAction = actionComp->GetCurrentAction();
	if (!currentAction) return;

	UCAction_ComboAttack* action_ComboAttack = Cast<UCAction_ComboAttack>(currentAction);
	if (!action_ComboAttack) return;

	switch (NotifyType)
	{
	case EPreInputNotifyType::Enabled:
	{
		// FLog::Log(TEXT("[AnimNotify|PreInput] Enabled"));
		action_ComboAttack->EnablePreInput();
		break;
	}
	case EPreInputNotifyType::Disabled:
	{
		// FLog::Log(TEXT("[AnimNotify|PreInput] Disabled"));
		action_ComboAttack->DisablePreInput();
		break;
	}
	}
}