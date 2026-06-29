#include "Notify/CAnimNotify_EnterAliveState.h"
#include "ProjectGlobal.h"

#include "Component/CHealthComponent.h"

UCAnimNotify_EnterAliveState::UCAnimNotify_EnterAliveState()
{
}

FString UCAnimNotify_EnterAliveState::GetNotifyName_Implementation() const
{
	return TEXT("Enter Alive State");
}

void UCAnimNotify_EnterAliveState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCHealthComponent* healthComp = GetHealthComponent(MeshComp);
	if (!IsValid(healthComp)) return;

	healthComp->HandleDeadStateNotify(EDeadState::Alive);
}
