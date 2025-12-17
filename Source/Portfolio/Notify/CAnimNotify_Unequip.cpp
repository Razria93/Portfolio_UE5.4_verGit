#include "Notify/CAnimNotify_Unequip.h"
#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"
#include "Weapon/CEquipment.h"

UCAnimNotify_Unequip::UCAnimNotify_Unequip()
{
}

FString UCAnimNotify_Unequip::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Unequip");
}

void UCAnimNotify_Unequip::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UCWeaponComponent* weaponComp = GetWeaponComponent(MeshComp);

	if (!weaponComp)
		return;

	UCEquipment* equipment = weaponComp->GetEquipment();

	if (!equipment)
		return;

	switch (FlowType)
	{
	case EAnimNotifyFlow::Begin:
	{
		equipment->Begin_Unequip();
		return;
	}

	case EAnimNotifyFlow::End:
	{
		equipment->End_Unequip();
		return;
	}
	} // switch
}