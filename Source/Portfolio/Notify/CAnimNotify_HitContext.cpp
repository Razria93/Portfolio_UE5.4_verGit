#include "Notify/CAnimNotify_HitContext.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction.h"

UCAnimNotify_HitContext::UCAnimNotify_HitContext()
{
}

FString UCAnimNotify_HitContext::GetNotifyName_Implementation() const
{
	switch (NotifyType)
	{
	case EHitContextNotifyType::Push:
		return TEXT("HitContext(Push)");

	case EHitContextNotifyType::Clear:
		return TEXT("HitContext(Clear)");

	default:
		return TEXT("HitContext");
	}
}

void UCAnimNotify_HitContext::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!actionComp) return;

	UCAction* currentAction = actionComp->GetCurrentAction();
	if (!currentAction) return;

	if (!CanProcessActionNotify(currentAction)) return;

	switch (NotifyType)
	{
	case EHitContextNotifyType::Push: currentAction->PushHitContext(); return;
	case EHitContextNotifyType::Clear: currentAction->ClearHitContext(); return;
	}
}
