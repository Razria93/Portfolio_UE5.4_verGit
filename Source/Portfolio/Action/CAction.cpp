#include "Action/CAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CActionFeedbackComponent.h"

#include "Interface/HitContextProducer.h"

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

	ActionComp_Cached = Cast<UCActionComponent>(OwnerCharacter_Injected->GetComponentByClass(UCActionComponent::StaticClass()));							// TODO: Refactor Interface
	check(ActionComp_Cached);

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

void UCAction::PlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bIsAction = true;

	StateComp_Cached->SetActionState();

	// NOTE: To be implemented detail by derived classes
}

void UCAction::BeginPlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bBeginAction = true;

	// NOTE: To be implemented detail by derived classes
}

void UCAction::EndPlayAction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bIsAction = false;
	bBeginAction = false;

	StateComp_Cached->SetIdleState();

	// NOTE: To be implemented detail by derived classes
}

void UCAction::NotifyActionTrailBegin()
{
	FActionContext actionContext;
	actionContext.CurrentActionType = ActionType;
	actionContext.ActionIndex = INDEX_NONE;

	RequestPlayActionFeedback(actionContext, EActionFeedbackPhase::TrailWindowBegin);
}

void UCAction::NotifyActionTrailEnd()
{
	FActionContext actionContext;
	actionContext.CurrentActionType = ActionType;
	actionContext.ActionIndex = INDEX_NONE;

	RequestPlayActionFeedback(actionContext, EActionFeedbackPhase::TrailWindowEnd);
}

void UCAction::PushContextToAttachment(const FActionContext& InActionContext)
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->PushContextToAttachment(InActionContext);
}

void UCAction::ClearContextToAttachment()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->ClearContextToAttachment();
}

void UCAction::RequestPlayActionFeedback(const FActionContext& InActionContext, EActionFeedbackPhase InPhase)
{
	if (!IsValid(ActionFeedbackComp_Cached)) return;
	if (!IsValid(WeaponComp_Cached)) return;

	const FApplyDamageSpecKey applyDamageSpecKey = WeaponComp_Cached->BuildApplyDamageSpecKey(InActionContext);
	ActionFeedbackComp_Cached->PlayActionFeedback(applyDamageSpecKey, InPhase);
}