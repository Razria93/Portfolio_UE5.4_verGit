#include "Action/CAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CActionFeedbackComponent.h"

#include "Type/CWeaponStructure.h"

void UCAction::InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData>& InActionDatas)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	ActionType = InActionType;
	ActionDatas_Injected = InActionDatas;

	if (!IsValid(OwnerCharacter_Injected)) return;

	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerCharacter_Injected->GetComponentByClass(UCWeaponComponent::StaticClass()));							// TODO: Refactor Interface
	check(WeaponComp_Cached);

	ActionFeedbackComp_Cached = Cast<UCActionFeedbackComponent>(OwnerCharacter_Injected->GetComponentByClass(UCActionFeedbackComponent::StaticClass()));	// TODO: Refactor Interface
	check(ActionFeedbackComp_Cached);
}

EActionType UCAction::GetActionType() const
{
	return ActionType;
}

void UCAction::SetActionType(EActionType InActionType)
{
	ActionType = InActionType;
}

EActionExecutionDecision UCAction::DecideExecution(const FActionExecutionQuery& InActionExecuteQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EActionExecutionDecision::Reject;

	if (InActionExecuteQuery.ExecutionState == EExecutionState::Idle && InActionExecuteQuery.CurrentActionType == EActionType::Idle)
	{
		return EActionExecutionDecision::Start;
	}

	return EActionExecutionDecision::Reject;
}

bool UCAction::Start()
{
	if (!IsValid(OwnerCharacter_Injected)) return false;

	bIsAction = true;

	RequestFeedback(EActionFeedbackTiming::ActionStart, NAME_None);

	return true;
}

bool UCAction::ApplyChain(const FActionExecutionQuery& InActionExecuteQuery)
{
	return false;
}

void UCAction::Complete()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	RequestFeedback(EActionFeedbackTiming::ActionEnd, NAME_None);

	bIsAction = false;
}

void UCAction::PushHitContext()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->PushContext(BuildActionContext());
}

void UCAction::ClearHitContext()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->ClearContext();
}

void UCAction::RequestFeedback(EActionFeedbackTiming InActionFeedbackTiming, FName InTriggerKey) const
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(ActionFeedbackComp_Cached)) return;

	ActionFeedbackComp_Cached->PlayFeedback(BuildFeedbackRequest(InActionFeedbackTiming, InTriggerKey));
}

FActionContext UCAction::BuildActionContext() const
{
	FActionContext actionContext;

	actionContext.ActionType = ActionType;
	actionContext.ActionIndex = INDEX_NONE;

	return actionContext;
}

FActionFeedbackRequest UCAction::BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FActionFeedbackRequest ActionFeedbackRequest;

	ActionFeedbackRequest.ActionFeedbackKey.ActionType = ActionType;
	ActionFeedbackRequest.ActionFeedbackKey.ActionIndex = INDEX_NONE;
	ActionFeedbackRequest.ActionFeedbackTiming = InTiming;
	ActionFeedbackRequest.TriggerKey = InTriggerKey;

	return ActionFeedbackRequest;
}
