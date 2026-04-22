#include "Notify/CAnimNotifyState_ActionFeedback.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction.h"

#include "Type/CWeaponStructure.h"

UCAnimNotifyState_ActionFeedback::UCAnimNotifyState_ActionFeedback()
{
}

FString UCAnimNotifyState_ActionFeedback::GetNotifyName_Implementation() const
{
	return TriggerKey.IsNone() ? TEXT("ActionFeedback(Window)") : FString::Printf(TEXT("ActionFeedback(Window: %s)"), *TriggerKey.ToString());
}

void UCAnimNotifyState_ActionFeedback::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!IsValid(actionComp)) return;

	UCAction* curAction = actionComp->GetCurrentAction();
	if (!IsValid(curAction)) return;

	curAction->RequestFeedback(EActionFeedbackTiming::TriggerWindowBegin, TriggerKey);
}

void UCAnimNotifyState_ActionFeedback::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!IsValid(actionComp)) return;

	UCAction* curAction = actionComp->GetCurrentAction();
	if (!IsValid(curAction)) return;

	curAction->RequestFeedback(EActionFeedbackTiming::TriggerWindowEnd, TriggerKey);
}
