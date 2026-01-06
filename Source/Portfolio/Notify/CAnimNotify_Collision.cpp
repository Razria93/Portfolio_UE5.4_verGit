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

	UObject* uobject = weaponComp->GetAttachment();

	ACAttachment* attachment = Cast<ACAttachment>(uobject);

	if (!attachment) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: attachment->CollisionEnabled(CollisionName); return;
	case EAnimNotifyFlow::End: attachment->CollisionDisabled(); return;
	}
}
