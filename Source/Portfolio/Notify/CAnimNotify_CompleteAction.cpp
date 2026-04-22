#include "Notify/CAnimNotify_CompleteAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"

UCAnimNotify_CompleteAction::UCAnimNotify_CompleteAction()
{
}

FString UCAnimNotify_CompleteAction::GetNotifyName_Implementation() const
{
	return TEXT("Complete Action");
}

void UCAnimNotify_CompleteAction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!actionComp) return;

	actionComp->CompleteAction();
}
