#include "Notify/CAnimNotify_EnterAliveState.h"
#include "ProjectGlobal.h"

#include "GameFramework/Actor.h"
#include "Component/CHealthComponent.h"

UCAnimNotify_EnterAliveState::UCAnimNotify_EnterAliveState()
{
}

FString UCAnimNotify_EnterAliveState::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Enter Alive State");
}

void UCAnimNotify_EnterAliveState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	if (!IsValid(ownerActor)) return;

	UCHealthComponent* healthComp = Cast<UCHealthComponent>(ownerActor->GetComponentByClass(UCHealthComponent::StaticClass()));
	if (!IsValid(healthComp)) return;

	healthComp->EnterAliveState();
}
