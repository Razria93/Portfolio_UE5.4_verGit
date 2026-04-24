#include "Action/CAction_Equip.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

#include "Type/CWeaponStructure.h"

EActionExecutionDecision UCAction_Equip::DecideExecution(const FActionExecutionQuery & InActionExecuteQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EActionExecutionDecision::Reject;
	if (!IsValid(WeaponComp_Cached)) return EActionExecutionDecision::Reject;

	if (!WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed)) return EActionExecutionDecision::Reject;

	if (!ActionDatas_Injected.IsValidIndex(0)) return EActionExecutionDecision::Reject;
	if (!IsValid(ActionDatas_Injected[0].Montage)) return EActionExecutionDecision::Reject;
	
	if (InActionExecuteQuery.ExecutionState == EExecutionState::Idle && InActionExecuteQuery.CurrentActionType == EActionType::Idle)
	{
		return EActionExecutionDecision::Start;
	}

	return EActionExecutionDecision::Reject;
}

bool UCAction_Equip::Start()
{
	if (!Super::Start()) return false;

	ActionDatas_Injected[0].BeginPlayMontage(OwnerCharacter_Injected);

	return true;
}

void UCAction_Equip::Complete()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	if (ActionDatas_Injected.IsValidIndex(0) && IsValid(ActionDatas_Injected[0].Montage))
	{
		ActionDatas_Injected[0].EndPlayMontage(OwnerCharacter_Injected);
	}

	Super::Complete();	// bIsAction, bBeginAction = false
}

void UCAction_Equip::AttachWeapon()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->AttachWeaponToHand();
	WeaponComp_Cached->CommitEquipWeapon();
}
