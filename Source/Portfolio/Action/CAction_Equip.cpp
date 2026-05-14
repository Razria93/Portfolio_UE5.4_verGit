#include "Action/CAction_Equip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

EActionLocalLevelDecision UCAction_Equip::ResolveLocalLevelDecision(const FActionLocalLevelQuery& InQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EActionLocalLevelDecision::Reject;
	if (!IsValid(WeaponComp_Cached)) return EActionLocalLevelDecision::Reject;

	if (!WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed))
	{
		return EActionLocalLevelDecision::Reject;
	}

	const bool bIsIdle = InQuery.ExecutionState == EExecutionState::Idle;
	const bool bIsActiveAction = InQuery.bIsActiveAction;

	if (bIsIdle && !bIsActiveAction)
	{
		return EActionLocalLevelDecision::Start;
	}

	return EActionLocalLevelDecision::Reject;
}

void UCAction_Equip::AttachWeapon()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->AttachWeaponToHand();
	WeaponComp_Cached->CommitEquipWeapon();
}
