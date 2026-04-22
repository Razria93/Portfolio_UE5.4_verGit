#include "Notify/CAnimNotify_PreInput.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction_ComboAttack.h"

UCAnimNotify_PreInput::UCAnimNotify_PreInput()
{
}

FString UCAnimNotify_PreInput::GetNotifyName_Implementation() const
{
	switch (NotifyType)
	{
	case EPreInputNotifyType::Enabled:
		return TEXT("PreInput(Enabled)");

	case EPreInputNotifyType::Disabled:
		return TEXT("PreInput(Disabled)");

	default:
		return TEXT("PreInput");
	}
}

void UCAnimNotify_PreInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!actionComp) return;

	UCAction* curAction = actionComp->GetCurrentAction();
	if (!curAction) return;

	UCAction_ComboAttack* action_ComboAttack = Cast<UCAction_ComboAttack>(curAction);
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