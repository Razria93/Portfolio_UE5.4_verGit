#include "Notify/CAnimNotify_EnterDeadState.h"
#include "ProjectGlobal.h"

#include "Component/CHealthComponent.h"

UCAnimNotify_EnterDeadState::UCAnimNotify_EnterDeadState()
{
}

FString UCAnimNotify_EnterDeadState::GetNotifyName_Implementation() const
{
	return TEXT("Enter Dead State");
}

void UCAnimNotify_EnterDeadState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCHealthComponent* healthComp = GetHealthComponent(MeshComp);
	if (!IsValid(healthComp)) return;

	healthComp->HandleDeadStateNotify(EDeadState::Dead);
}
