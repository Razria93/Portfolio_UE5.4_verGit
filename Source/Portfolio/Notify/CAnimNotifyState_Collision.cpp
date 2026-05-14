#include "Notify/CAnimNotifyState_Collision.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CWeaponActor.h"

UCAnimNotifyState_Collision::UCAnimNotifyState_Collision()
{
}

FString UCAnimNotifyState_Collision::GetNotifyName_Implementation() const
{
	return CollisionName.IsNone() ? TEXT("Collision Window") : FString::Printf(TEXT("Collision Window: %s"), *CollisionName.ToString());
}

void UCAnimNotifyState_Collision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	ACWeaponActor* weaponActor = GetWeaponActor(MeshComp);
	if (!IsValid(weaponActor)) return;

	weaponActor->CollisionEnabled(CollisionName);
}

void UCAnimNotifyState_Collision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ACWeaponActor* weaponActor = GetWeaponActor(MeshComp);
	if (!IsValid(weaponActor)) return;

	weaponActor->CollisionDisabled();
}

ACWeaponActor* UCAnimNotifyState_Collision::GetWeaponActor(USkeletalMeshComponent* InMeshComp) const
{
	if (!IsValid(InMeshComp)) return nullptr;

	ACharacter* ownerCharacter = Cast<ACharacter>(InMeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return nullptr;

	UCWeaponComponent* weaponComp = ownerCharacter->FindComponentByClass<UCWeaponComponent>();
	if (!IsValid(weaponComp)) return nullptr;

	return weaponComp->GetWeaponActor();
}
