#include "Notify/CAnimNotify_Collision.h"
#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CAttachment.h"

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

	ACAttachment* attachment = weaponComp->GetAttachment();

	if (!attachment) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: attachment->OnCollision(CollisionName); return;
	case EAnimNotifyFlow::End: attachment->OffCollision(); return;
	}
}
