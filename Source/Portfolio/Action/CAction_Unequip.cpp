#include "Action/CAction_Unequip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

EExecutionDecision UCAction_Unequip::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EExecutionDecision::Reject;
	if (!IsValid(WeaponComp_Cached)) return EExecutionDecision::Reject;
	if (!InQuery.IncomingPart.IsActionParticipant()) return EExecutionDecision::Reject;

	if (!InQuery.Snapshot.IsIdle()) return EExecutionDecision::Reject;
	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed)) return EExecutionDecision::Reject;

	return EExecutionDecision::Executable;
}

void UCAction_Unequip::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EActionNotifyCommand::Unequip:
		DetachWeapon();
		return;

	default:
		return;
	}
}

void UCAction_Unequip::DetachWeapon()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->AttachWeaponToHolster();
	WeaponComp_Cached->CommitUnequipWeapon();
}
