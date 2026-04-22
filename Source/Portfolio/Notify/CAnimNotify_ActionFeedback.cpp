#include "Notify/CAnimNotify_ActionFeedback.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction.h"

#include "Type/CWeaponStructure.h"

UCAnimNotify_ActionFeedback::UCAnimNotify_ActionFeedback()
{
}

FString UCAnimNotify_ActionFeedback::GetNotifyName_Implementation() const
{
	return TriggerKey.IsNone() ? TEXT("ActionFeedback(Point)") : FString::Printf(TEXT("ActionFeedback(Point: %s)"), *TriggerKey.ToString());
}

void UCAnimNotify_ActionFeedback::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!IsValid(actionComp)) return;

	UCAction* curAction = actionComp->GetCurrentAction();
	if (!IsValid(curAction)) return;

	curAction->RequestFeedback(EActionFeedbackTiming::TriggerOnce, TriggerKey);
}