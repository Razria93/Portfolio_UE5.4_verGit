#include "Notify/CAnimNotify_Equip.h"
#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CEquipment.h"

UCAnimNotify_Equip::UCAnimNotify_Equip()
{
}

FString UCAnimNotify_Equip::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Equip");
}

void UCAnimNotify_Equip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = GetWeaponComponent(MeshComp);

	if (!weaponComp) return;

	UCEquipment* equipment = weaponComp->GetEquipment();

	if (!equipment) return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin: equipment->Begin_Equip(); return;
	case EAnimNotifyFlow::End: equipment->End_Equip(); return;
	}
}
