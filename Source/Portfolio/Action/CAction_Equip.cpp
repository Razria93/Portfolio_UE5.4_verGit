#include "Action/CAction_Equip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

EExecutionDecision UCAction_Equip::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EExecutionDecision::Reject;
	if (!IsValid(WeaponComp_Cached)) return EExecutionDecision::Reject;
	if (!InQuery.IncomingPart.IsActionParticipant()) return EExecutionDecision::Reject;

	if (!InQuery.Snapshot.IsIdle()) return EExecutionDecision::Reject;
	if (!WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed)) return EExecutionDecision::Reject;

	return EExecutionDecision::Executable;
}

void UCAction_Equip::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EActionNotifyCommand::Equip:
		AttachWeapon();
		return;

	default:
		return;
	}
}

void UCAction_Equip::AttachWeapon()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->AttachWeaponToHand();
	WeaponComp_Cached->CommitEquipWeapon();
}
