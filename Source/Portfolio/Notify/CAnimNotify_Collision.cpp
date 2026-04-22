#include "Notify/CAnimNotify_Collision.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CWeaponActor.h"

UCAnimNotify_Collision::UCAnimNotify_Collision()
{
}

FString UCAnimNotify_Collision::GetNotifyName_Implementation() const
{
	switch (NotifyType)
	{
	case ECollisionNotifyType::Enabled:
		return TEXT("Collision(Enabled)");

	case ECollisionNotifyType::Disabled:
		return TEXT("Collision(Disabled)");

	default:
		return TEXT("Collision");
	}
}

void UCAnimNotify_Collision::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	ACharacter* ownerCharacter = Cast<ACharacter>(MeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return;

	UCWeaponComponent* weaponComp = ownerCharacter->FindComponentByClass<UCWeaponComponent>();
	if (!weaponComp) return;

	UObject* uobject = weaponComp->GetWeaponActor();
	ACWeaponActor* weaponActor = Cast<ACWeaponActor>(uobject);
	if (!weaponActor) return;

	switch (NotifyType)
	{
	case ECollisionNotifyType::Enabled: weaponActor->CollisionEnabled(CollisionName); return;
	case ECollisionNotifyType::Disabled: weaponActor->CollisionDisabled(); return;
	}
}
