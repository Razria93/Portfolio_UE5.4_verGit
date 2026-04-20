#include "Action/CAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CActionFeedbackComponent.h"

#include "Type/CWeaponStructure.h"

void UCAction::InitializeAction(ACharacter* InOwnerCharacter, EActionType InActionType, const TArray<FActionData> InActionDatas)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	ActionType = InActionType;
	ActionDatas_Injected = InActionDatas;

	if (!IsValid(OwnerCharacter_Injected)) return;

	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerCharacter_Injected->GetComponentByClass(UCWeaponComponent::StaticClass()));							// TODO: Refactor Interface
	check(WeaponComp_Cached);

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Injected->GetComponentByClass(UCStateComponent::StaticClass()));								// TODO: Refactor Interface
	check(StateComp_Cached);

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

bool UCAction::PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return false;

	bIsAction = true;

	StateComp_Cached->SetActionState();

	// NOTE: To be implemented detail by derived classes
	return true;
}

void UCAction::BeginPlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bBeginAction = true;

	RequestPlayActionFeedback(EActionFeedbackTiming::ActionStart);

	// NOTE: To be implemented detail by derived classes
}

void UCAction::EndPlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	RequestPlayActionFeedback(EActionFeedbackTiming::ActionEnd);

	bIsAction = false;
	bBeginAction = false;

	StateComp_Cached->SetIdleState();

	// NOTE: To be implemented detail by derived classes
}


FActionContext UCAction::BuildActionContext() const
{
	FActionContext actionContext;

	actionContext.CurrentActionType = ActionType;
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

void UCAction::PushContextToWeaponActor(const FActionContext& InActionContext)
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->PushContextToWeaponActor(InActionContext);
}

void UCAction::ClearContextToWeaponActor()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->ClearContextToWeaponActor();
}

void UCAction::RequestPlayActionFeedback(EActionFeedbackTiming InActionFeedbackTiming, FName InTriggerKey) const
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(ActionFeedbackComp_Cached)) return;

	ActionFeedbackComp_Cached->PlayActionFeedback(BuildActionFeedbackRequest(InActionFeedbackTiming, InTriggerKey));
}
