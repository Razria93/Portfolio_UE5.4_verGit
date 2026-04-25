#include "Notify/CAnimNotify_AdvanceCombo.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction_ComboAttack.h"

UCAnimNotify_AdvanceCombo::UCAnimNotify_AdvanceCombo()
{
}

FString UCAnimNotify_AdvanceCombo::GetNotifyName_Implementation() const
{
	return TEXT("Advance Combo");
}

void UCAnimNotify_AdvanceCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!IsValid(actionComp)) return;

	UCAction* currentAction = actionComp->GetCurrentAction();
	if (!IsValid(currentAction)) return;

	UCAction_ComboAttack* comboAttack = Cast<UCAction_ComboAttack>(currentAction);
	if (!IsValid(comboAttack)) return;

	comboAttack->AdvanceCombo();
}
