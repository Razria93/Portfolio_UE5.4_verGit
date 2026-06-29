#include "Notify/CAnimNotify_HealthBase.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CHealthComponent.h"

UCAnimNotify_HealthBase::UCAnimNotify_HealthBase()
{
}

UCHealthComponent* UCAnimNotify_HealthBase::GetHealthComponent(USkeletalMeshComponent* InMeshComp) const
{
	if (!IsValid(InMeshComp)) return nullptr;

	ACharacter* ownerCharacter = Cast<ACharacter>(InMeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return nullptr;

	return ownerCharacter->FindComponentByClass<UCHealthComponent>();
}
