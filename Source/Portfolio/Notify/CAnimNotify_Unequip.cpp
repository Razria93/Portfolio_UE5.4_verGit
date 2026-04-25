#include "Notify/CAnimNotify_Unequip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction_Unequip.h"

UCAnimNotify_Unequip::UCAnimNotify_Unequip()
{
}

FString UCAnimNotify_Unequip::GetNotifyName_Implementation() const
{
	return TEXT("Unequip");
}

void UCAnimNotify_Unequip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!actionComp) return;

	UCAction* currentAction = actionComp->GetCurrentAction();
	if (!currentAction) return;

	UCAction_Unequip* currentaction_Unequip = Cast<UCAction_Unequip>(currentAction);
	if (!currentaction_Unequip) return;

	if (!CanProcessActionNotify(currentaction_Unequip)) return;

	currentaction_Unequip->DetachWeapon();
}
