#include "Action/CAction_ComboAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

#include "Type/CWeaponStructure.h"

void UCAction_ComboAttack::InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData> InActionDatas)
{
	Super::InitializeAction(InOwnerCharacter, InActionType, InActionDatas);

	ActionIndex = 0;

	bEnablePreInput = false;
	bExistPreInput = false;
}

bool UCAction_ComboAttack::CanStart() const
{
	if (!Super::CanStart()) return false;
	if (!IsValid(WeaponComp_Cached)) return false;

	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed)) return false;

	if (!ActionDatas_Injected.IsValidIndex(ActionIndex)) return false;
	if (!IsValid(ActionDatas_Injected[ActionIndex].Montage)) return false;

	return true;
}

bool UCAction_ComboAttack::Start()
{
	// [Re-Entry]
	if (bEnablePreInput)
	{
		bEnablePreInput = false;
		bExistPreInput = true;

		FLog::Log(TEXT("[ComboAttack|Start] Buffered PreInput"));
		return true;
	}

	// [First-Entry]
	if (!CanStart()) return false;
	if (!Super::Start()) return false;

	ActionDatas_Injected[ActionIndex].BeginPlayMontage(OwnerCharacter_Injected);

	return true;
}

void UCAction_ComboAttack::Complete()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	if (ActionDatas_Injected.IsValidIndex(ActionIndex) && IsValid(ActionDatas_Injected[ActionIndex].Montage))
	{
		ActionDatas_Injected[ActionIndex].EndPlayMontage(OwnerCharacter_Injected);
	}

	Super::Complete();	// bIsAction, bBeginAction = false

	ActionIndex = 0;

	bEnablePreInput = false;
	bExistPreInput = false;
}

void UCAction_ComboAttack::OpenComboPreInput()
{
	bEnablePreInput = true;
}

void UCAction_ComboAttack::CloseComboPreInput()
{
	bEnablePreInput = false;
}

void UCAction_ComboAttack::AdvanceCombo()
{
	if (!bExistPreInput)
	{
		FLog::Log(TEXT("[ComboAttack|TryAdvanceCombo] No Buffered PreInput"));
		return;
	}

	bExistPreInput = false;

	const int32 nextActionIndex = ActionIndex + 1;

	if (!ActionDatas_Injected.IsValidIndex(nextActionIndex))
	{
		FLog::Log(FString::Printf(TEXT("[ComboAttack|AdvanceCombo] Invalid NextActionIndex | NextActionIndex = %d"), nextActionIndex));
		return;
	}

	if (!IsValid(ActionDatas_Injected[nextActionIndex].Montage))
	{
		FLog::Log(FString::Printf(TEXT("[ComboAttack|AdvanceCombo] Invalid Montage | NextActionIndex = %d"), nextActionIndex));
		return;
	}

	ActionIndex = nextActionIndex;

	FLog::Log(FString::Printf(TEXT("[ComboAttack|AdvanceCombo] AdvanceCombo | ActionIndex = %d"), ActionIndex));

	ActionDatas_Injected[ActionIndex].BeginPlayMontage(OwnerCharacter_Injected);
}

FActionContext UCAction_ComboAttack::BuildActionContext() const
{
	FActionContext actionContext;

	actionContext.ActionType = ActionType;
	actionContext.ActionIndex = ActionIndex;

	return actionContext;
}

FActionFeedbackRequest UCAction_ComboAttack::BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FActionFeedbackRequest actionFeedbackRequest;

	actionFeedbackRequest.ActionFeedbackKey.ActionType = ActionType;
	actionFeedbackRequest.ActionFeedbackKey.ActionIndex = ActionIndex;
	actionFeedbackRequest.ActionFeedbackTiming = InTiming;
	actionFeedbackRequest.TriggerKey = InTriggerKey;

	return actionFeedbackRequest;
}
