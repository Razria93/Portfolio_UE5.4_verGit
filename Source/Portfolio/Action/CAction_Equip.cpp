#include "Action/CAction_Equip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

FExecutionDecisionResult UCAction_Equip::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsValid(WeaponComp_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!IsIncomingActionType(InQuery, EActionType::Equip))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!CanResolveIndependentRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!WeaponComp_Injected->CheckCurrentWeaponType(EWeaponType::Unarmed))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Independent;
	return result;
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
	if (!IsValid(WeaponComp_Injected)) return;

	WeaponComp_Injected->AttachWeaponToHand();
	WeaponComp_Injected->CommitEquipWeapon();
}
