#include "Action/CAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CActionFeedbackComponent.h"

#include "Type/CWeaponStructure.h"

void UCAction::InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData>& InActionDatas)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	ActionType = InActionType;
	ActionDatas_Injected = InActionDatas;

	if (!IsValid(OwnerCharacter_Injected)) return;

	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerCharacter_Injected->GetComponentByClass(UCWeaponComponent::StaticClass()));
	check(WeaponComp_Cached);

	ActionComp_Cached = Cast<UCActionComponent>(OwnerCharacter_Injected->GetComponentByClass(UCActionComponent::StaticClass()));
	check(ActionComp_Cached);

	ActionFeedbackComp_Cached = Cast<UCActionFeedbackComponent>(OwnerCharacter_Injected->GetComponentByClass(UCActionFeedbackComponent::StaticClass()));
	check(ActionFeedbackComp_Cached);
}

EActionType UCAction::GetActionType() const
{
	return ActionType;
}

FActionContext UCAction::GetActionContext() const
{
	return BuildActionContext();
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
	EmitActionEvent(EActionEventType::ActionStarted, INDEX_NONE);
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
	EmitActionEvent(EActionEventType::ActionCompleted, INDEX_NONE);

	bIsAction = false;
}

void UCAction::Abort(EActionAbortReason InActionAbortReason)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	EmitActionEvent(EActionEventType::ActionAborted, INDEX_NONE);

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

	ActionFeedbackComp_Cached->PlayFeedback(BuildActionFeedbackRequest(InActionFeedbackTiming, InTriggerKey));
}

FActionContext UCAction::BuildActionContext() const
{
	FActionContext actionContext;

	actionContext.ActionType = ActionType;
	actionContext.ActionIndex = INDEX_NONE;

	return actionContext;
}

FActionFeedbackRequest UCAction::BuildActionFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FActionFeedbackRequest ActionFeedbackRequest;

	ActionFeedbackRequest.ActionFeedbackKey.ActionType = ActionType;
	ActionFeedbackRequest.ActionFeedbackKey.ActionIndex = INDEX_NONE;
	ActionFeedbackRequest.ActionFeedbackTiming = InTiming;
	ActionFeedbackRequest.TriggerKey = InTriggerKey;

	return ActionFeedbackRequest;
}

void UCAction::EmitActionEvent(EActionEventType InActionEventType, int32 InActionIndex) const
{
	if (!IsValid(ActionComp_Cached)) return;

	const FActionContext actionContext = BuildActionContext();
	const int32 actionIndex = (InActionIndex != INDEX_NONE) ? InActionIndex : actionContext.ActionIndex;

	ActionComp_Cached->BroadcastActionEvent(ActionType, actionIndex, InActionEventType);
}
