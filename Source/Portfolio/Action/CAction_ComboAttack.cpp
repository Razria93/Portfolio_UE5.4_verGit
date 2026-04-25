#include "Action/CAction_ComboAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"

#include "Type/CWeaponStructure.h"

void UCAction_ComboAttack::InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData>& InActionDatas)
{
	Super::InitializeAction(InOwnerCharacter, InActionType, InActionDatas);

	ActionIndex = 0;

	bEnablePreInput = false;
	bExistPreInput = false;
}

EActionExecutionDecision UCAction_ComboAttack::DecideExecution(const FActionExecutionQuery& InActionExecuteQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EActionExecutionDecision::Reject;
	if (!IsValid(WeaponComp_Cached)) return EActionExecutionDecision::Reject;

	if (WeaponComp_Cached->CheckCurrentWeaponType(EWeaponType::Unarmed)) return EActionExecutionDecision::Reject;

	const bool bFirstEntry = InActionExecuteQuery.ExecutionState == EExecutionState::Idle && InActionExecuteQuery.CurrentActionType == EActionType::Idle;

	if (bFirstEntry)
	{
		if (!ActionDatas_Injected.IsValidIndex(ActionIndex)) return EActionExecutionDecision::Reject;
		if (!IsValid(ActionDatas_Injected[ActionIndex].Montage)) return EActionExecutionDecision::Reject;

		return EActionExecutionDecision::Start;
	}

	const bool bCanChain = InActionExecuteQuery.CurrentActionType == ActionType && bEnablePreInput;

	if (bCanChain)
	{
		return EActionExecutionDecision::Chain;
	}

	return EActionExecutionDecision::Reject;
}

bool UCAction_ComboAttack::Start()
{
	if (!Super::Start()) return false;

	ActionDatas_Injected[ActionIndex].BeginPlayMontage(OwnerCharacter_Injected);

	return true;
}

bool UCAction_ComboAttack::ApplyChain(const FActionExecutionQuery& InActionExecuteQuery)
{
	if (InActionExecuteQuery.CurrentActionType != ActionType) return false;
	if (!bEnablePreInput) return false;

	bEnablePreInput = false;
	bExistPreInput = true;

	FLog::Log(TEXT("[ComboAttack|Chain] Buffered PreInput"));

	return true;
}

void UCAction_ComboAttack::Complete()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	if (ActionDatas_Injected.IsValidIndex(ActionIndex) && IsValid(ActionDatas_Injected[ActionIndex].Montage))
	{
		ActionDatas_Injected[ActionIndex].EndPlayMontage(OwnerCharacter_Injected);
	}

	Super::Complete();

	ActionIndex = 0;

	bEnablePreInput = false;
	bExistPreInput = false;
}

void UCAction_ComboAttack::Abort(EActionAbortReason InActionAbortReason)
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		Super::Abort(InActionAbortReason);
		return;
	}

	if (ActionDatas_Injected.IsValidIndex(ActionIndex) && IsValid(ActionDatas_Injected[ActionIndex].Montage))
	{
		ActionDatas_Injected[ActionIndex].EndPlayMontage(OwnerCharacter_Injected);
	}

	Super::Abort(InActionAbortReason);

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

	FLog::Log(FString::Printf(TEXT("[ComboAttack|AdvanceCombo] Advance Combo | ActionIndex = %d"), ActionIndex));

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
