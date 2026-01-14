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

void UCAction_ComboAttack::PlayAction()
{
	// [Re-call] Convert 're-invoked PlayAction()' into 'buffered pre-input'
	if (bEnablePreInput)
	{
		bEnablePreInput = false; // Enabled by CAnimNotify_ComboEnable
		bExistPreInput = true;	 // Mark pre-input for next combo step

		return;
	}

	// [First-call] Validate execution conditions & Execute first combo action
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached) || !IsValid(WeaponComp_Cached)) return;
	if (WeaponComp_Cached->CheckCurAttachmentType(EAttachmentType::Unarmed)) return;
	if (!StateComp_Cached->CheckCurStateType(EStateType::Idle)) return;
	if (ActionDatas_Injected.Num() <= 0) return;

	Super::PlayAction();		// bIsAction = true

	if (!IsValid(ActionDatas_Injected[ActionIndex].Montage)) return;
	ActionDatas_Injected[ActionIndex].BeginPlayMontage(OwnerCharacter_Injected);
}

void UCAction_ComboAttack::BeginPlayAction()
{
	Super::BeginPlayAction();	// bBeginAction = true

	FActionContext actionContext;
	actionContext.CurrentActionType = ActionType;
	actionContext.ActionIndex = ActionIndex;

	PushContextToAttachment(actionContext);
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

	ClearContextToAttachment();
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

		if (!IsValid(ActionDatas_Injected[ActionIndex].Montage)) return;
		ActionDatas_Injected[ActionIndex].BeginPlayMontage(OwnerCharacter_Injected);

		FActionContext actionContext;
		actionContext.CurrentActionType = ActionType;
		actionContext.ActionIndex = ActionIndex; // Increased ActionIndex

		PushContextToAttachment(actionContext);
	}
}