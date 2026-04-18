#include "Notify/CAnimNotifyState_ActionFeedback.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionFeedbackComponent.h"

#include "Interface/ActionFeedbackRequestProvider.h"

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

	IActionFeedbackRequestProvider* requestProvider = Cast<IActionFeedbackRequestProvider>(ownerCharacter);
	if (!requestProvider) return;

	UCActionFeedbackComponent* actionFeedbackComp = ownerCharacter->FindComponentByClass<UCActionFeedbackComponent>();
	if (!IsValid(actionFeedbackComp)) return;

	FActionFeedbackRequest actionFeedbackRequest;
	if (!requestProvider->BuildActionFeedbackRequest(EActionFeedbackTiming::TriggerWindowBegin, TriggerKey, actionFeedbackRequest)) return;

	FLog::Log(FString::Printf(TEXT("[ActionFeedback_NotifyBegin] ActionFeedbackTiming = %s | TriggerKey = %s"), *UEnum::GetValueAsString(actionFeedbackRequest.ActionFeedbackTiming), *actionFeedbackRequest.TriggerKey.ToString()));

	actionFeedbackComp->PlayActionFeedback(actionFeedbackRequest);
}

void UCAnimNotifyState_ActionFeedback::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	IActionFeedbackRequestProvider* requestProvider = Cast<IActionFeedbackRequestProvider>(ownerCharacter);
	if (!requestProvider) return;

	UCActionFeedbackComponent* actionFeedbackComp = ownerCharacter->FindComponentByClass<UCActionFeedbackComponent>();
	if (!IsValid(actionFeedbackComp)) return;

	FActionFeedbackRequest actionFeedbackRequest;
	if (!requestProvider->BuildActionFeedbackRequest(EActionFeedbackTiming::TriggerWindowEnd, TriggerKey, actionFeedbackRequest)) return;

	FLog::Log(FString::Printf(TEXT("[ActionFeedback_NotifyEnd] ActionFeedbackTiming = %s | TriggerKey = %s"), *UEnum::GetValueAsString(actionFeedbackRequest.ActionFeedbackTiming), *actionFeedbackRequest.TriggerKey.ToString()));

	actionFeedbackComp->PlayActionFeedback(actionFeedbackRequest);

}
