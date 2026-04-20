#include "Notify/CAnimNotify_Collision.h"
#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CWeaponActor.h"

UCAnimNotify_Collision::UCAnimNotify_Collision()
{
}

FString UCAnimNotify_Collision::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Collision");
}

void UCAnimNotify_Collision::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = GetWeaponComponent(MeshComp);

	if (!weaponComp) return;

	UObject* uobject = weaponComp->GetWeaponActor();

	ACWeaponActor* weaponActor = Cast<ACWeaponActor>(uobject);

	if (!weaponActor) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: weaponActor->CollisionEnabled(CollisionName); return;
	case EAnimNotifyFlow::End: weaponActor->CollisionDisabled(); return;
	}
}
