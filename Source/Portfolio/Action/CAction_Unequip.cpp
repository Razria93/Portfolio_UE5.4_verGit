#include "Action/CAction_Unequip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

#include "Type/CWeaponStructure.h"

EActionExecutionDecision UCAction_Unequip::DecideExecution(const FActionExecutionQuery& InActionExecuteQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EActionExecutionDecision::Reject;
	if (!IsValid(WeaponComp_Cached)) return EActionExecutionDecision::Reject;

	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed)) return EActionExecutionDecision::Reject;

	if (!ActionDatas_Injected.IsValidIndex(0)) return EActionExecutionDecision::Reject;
	if (!IsValid(ActionDatas_Injected[0].Montage)) return EActionExecutionDecision::Reject;

	if (InActionExecuteQuery.ExecutionState == EExecutionState::Idle && InActionExecuteQuery.CurrentActionType == EActionType::Idle)
	{
		return EActionExecutionDecision::Start;
	}

	return EActionExecutionDecision::Reject;
}

bool UCAction_Unequip::Start()
{
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

	Super::Complete();
}

void UCAction_Unequip::Abort(EActionAbortReason InActionAbortReason)
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		Super::Abort(InActionAbortReason);
		return;
	}

	if (ActionDatas_Injected.IsValidIndex(0) && IsValid(ActionDatas_Injected[0].Montage))
	{
		ActionDatas_Injected[0].EndPlayMontage(OwnerCharacter_Injected);
	}

	Super::Abort(InActionAbortReason);
}

void UCAction_Unequip::DetachWeapon()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->AttachWeaponToHolster();
	WeaponComp_Cached->CommitUnequipWeapon();
}
