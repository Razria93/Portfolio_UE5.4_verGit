#include "Notify/CAnimNotify_ActionFeedback.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionFeedbackComponent.h"
#include "Interface/ActionFeedbackRequestProvider.h"

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

	IActionFeedbackRequestProvider* requestProvider = Cast<IActionFeedbackRequestProvider>(ownerCharacter);
	if (!requestProvider) return;

	UCActionFeedbackComponent* actionFeedbackComp = ownerCharacter->FindComponentByClass<UCActionFeedbackComponent>();
	if (!IsValid(actionFeedbackComp)) return;

	FActionFeedbackRequest actionFeedbackRequest;
	if (!requestProvider->BuildActionFeedbackRequest(EActionFeedbackTiming::TriggerOnce, TriggerKey, actionFeedbackRequest)) return;

	FLog::Log(FString::Printf(TEXT("[ActionFeedback_NotifyPoint]  ActionFeedbackTiming = %s | TriggerKey = %s"), *UEnum::GetValueAsString(EActionFeedbackTiming::TriggerOnce), *TriggerKey.ToString()));

	actionFeedbackComp->PlayActionFeedback(actionFeedbackRequest);
}