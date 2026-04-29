#include "Notify/CAnimNotify_EnterDeadState.h"
#include "ProjectGlobal.h"

#include "GameFramework/Actor.h"
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

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	if (!IsValid(ownerActor)) return;

	UCHealthComponent* healthComp = Cast<UCHealthComponent>(ownerActor->GetComponentByClass(UCHealthComponent::StaticClass()));
	if (!IsValid(healthComp)) return;

	healthComp->EnterDeadState();
}
