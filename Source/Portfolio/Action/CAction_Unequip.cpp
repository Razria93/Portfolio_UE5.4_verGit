#include "Action/CAction_Unequip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

#include "Type/CWeaponStructure.h"

bool UCAction_Unequip::CanStart() const
{
	if (!Super::CanStart()) return false;
	if (!IsValid(WeaponComp_Cached)) return false;

	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed)) return false;

	if (!ActionDatas_Injected.IsValidIndex(0)) return false;
	if (!IsValid(ActionDatas_Injected[0].Montage)) return false;

	return true;
}

bool UCAction_Unequip::Start()
{
	if (!CanStart()) return false;
	if (!Super::Start()) return false;

	ActionDatas_Injected[0].BeginPlayMontage(OwnerCharacter_Injected);

	return true;
}

void UCAction_Unequip::Complete()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	if (ActionDatas_Injected.IsValidIndex(0) && IsValid(ActionDatas_Injected[0].Montage))
	{
		ActionDatas_Injected[0].EndPlayMontage(OwnerCharacter_Injected);
	}

	Super::Complete();	// bIsAction, bBeginAction = false
}

void UCAction_Unequip::DetachWeapon()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->AttachWeaponToHolster();
	WeaponComp_Cached->CommitUnequipWeapon();
}
