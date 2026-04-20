#include "Action/CAction_ComboAttack.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"

#include "Type/CWeaponStructure.h"
#include "Type/CStateStructure.h"

void UCAction_ComboAttack::InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData> InActionDatas)
{
	Super::InitializeAction(InOwnerCharacter, InActionType, InActionDatas);

	ActionIndex = 0;

	bEnablePreInput = false;
	bExistPreInput = false;
}

void UCAction_ComboAttack::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

bool UCAction_ComboAttack::PlayAction()
{
	// [Re-call] Convert 're-invoked PlayAction()' into 'buffered pre-input'
	if (bEnablePreInput)
	{
		bEnablePreInput = false; // Enabled by CAnimNotify_ComboEnable
		bExistPreInput = true;	 // Mark pre-input for next combo step

		FLog::Log(TEXT("[ComboAttack|PlayAction] Buffered PreInput"));
		return true;
	}

	// [First-call] Validate execution conditions & Execute first combo action
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached) || !IsValid(WeaponComp_Cached)) return false;
	if (WeaponComp_Cached->CheckCurWeaponActorType(EWeaponActorType::Unarmed)) return false;
	if (!StateComp_Cached->CheckCurExecutionState(EExecutionState::Idle)) return false;
	if (ActionDatas_Injected.Num() <= 0) return false;

	if (!Super::PlayAction()) return false;

	if (!IsValid(ActionDatas_Injected[ActionIndex].Montage)) return false;

	ActionDatas_Injected[ActionIndex].BeginPlayMontage(OwnerCharacter_Injected);
	return true;
}

void UCAction_ComboAttack::BeginPlayAction()
{
	Super::BeginPlayAction();	// bBeginAction = true

	if (!IsValid(OwnerCharacter_Injected)) return;

	FActionContext actionContext = BuildActionContext();

	PushContextToWeaponActor(actionContext);
}

void UCAction_ComboAttack::EndPlayAction()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	Super::EndPlayAction();	// bIsAction, bBeginAction = false

	const int32 num = ActionDatas_Injected.Num();

	if (num > 0 && ActionIndex >= 0 && ActionIndex < num)
	{
		if (IsValid(ActionDatas_Injected[ActionIndex].Montage))
			ActionDatas_Injected[ActionIndex].EndPlayMontage(OwnerCharacter_Injected);
	}

	ActionIndex = 0;

	bEnablePreInput = false;
	bExistPreInput = false;

	ClearContextToWeaponActor();
}

void UCAction_ComboAttack::NextPlayAction()
{
	if (bExistPreInput)
	{
		Super::NextPlayAction();

		bExistPreInput = false;

		const int32 num = ActionDatas_Injected.Num();
		if (num <= 0) return;

		const int32 nextActionIndex = ActionIndex + 1;
		if (nextActionIndex >= num) return;

		ActionIndex = nextActionIndex;

		FLog::Log(FString::Printf(TEXT("[ComboAttack|NextPlayAction] AdvanceCombo | ActionIndex = %d"), ActionIndex));

		if (!IsValid(ActionDatas_Injected[ActionIndex].Montage)) return;
		ActionDatas_Injected[ActionIndex].BeginPlayMontage(OwnerCharacter_Injected);

		FActionContext actionContext;
		actionContext.CurrentActionType = ActionType;
		actionContext.ActionIndex = ActionIndex; // Increased ActionIndex

		PushContextToWeaponActor(actionContext);
	}
	else
	{
		FLog::Log(TEXT("[ComboAttack|NextPlayAction] No Buffered PreInput"));
	}
}

FActionContext UCAction_ComboAttack::BuildActionContext() const
{
	FActionContext actionContext;

	actionContext.CurrentActionType = ActionType;
	actionContext.ActionIndex = ActionIndex;

	return actionContext;
}

FActionFeedbackRequest UCAction_ComboAttack::BuildActionFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FActionFeedbackRequest actionFeedbackRequest;

	actionFeedbackRequest.ActionFeedbackKey.ActionType = ActionType;
	actionFeedbackRequest.ActionFeedbackKey.ActionIndex = ActionIndex;
	actionFeedbackRequest.ActionFeedbackTiming = InTiming;
	actionFeedbackRequest.TriggerKey = InTriggerKey;

	return actionFeedbackRequest;
}
