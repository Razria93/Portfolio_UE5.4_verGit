#include "Action/CAction_Unequip.h"

#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"

#include "GameFramework/Character.h"

// Decision

FExecutionDecisionResult UCAction_Unequip::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
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

	if (!IsIncomingActionType(InQuery, EActionType::Unequip))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!CanResolveIndependentRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (WeaponComp_Injected->CheckCurrentWeaponType(EWeaponType::Unarmed))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Independent;
	return result;
}

// Notify

void UCAction_Unequip::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EActionNotifyCommand::CommitUnequipWeapon:
		if (IsValid(WeaponComp_Injected))
		{
			WeaponComp_Injected->CommitUnequipWeapon();
		}
		return;

	default:
		return;
	}
}
