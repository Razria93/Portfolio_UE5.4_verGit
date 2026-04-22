#include "Notify/CAnimNotify_Equip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction_Equip.h"

UCAnimNotify_Equip::UCAnimNotify_Equip()
{
}

FString UCAnimNotify_Equip::GetNotifyName_Implementation() const
{
	return TEXT("Equip");
}

void UCAnimNotify_Equip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!actionComp) return;

	UCAction_Equip* action_Equip = Cast<UCAction_Equip>(actionComp->GetCurrentAction());
	if (!action_Equip) return;

	action_Equip->AttachWeapon();
}
