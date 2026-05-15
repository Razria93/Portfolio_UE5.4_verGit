#include "Action/CAction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

#include "Component/CWeaponComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CActionFeedbackComponent.h"

void UCAction::InitializeAction(ACharacter* InOwnerCharacter, UCActionComponent* InOwnerActionComp)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	OwnerActionComp_Injected = InOwnerActionComp;

	if (!IsValid(OwnerCharacter_Injected)) return;

	WeaponComp_Cached = OwnerCharacter_Injected->FindComponentByClass<UCWeaponComponent>();
	check(WeaponComp_Cached);

	ActionFeedbackComp_Cached = OwnerCharacter_Injected->FindComponentByClass<UCActionFeedbackComponent>();
	check(ActionFeedbackComp_Cached);
}

EActionLocalLevelDecision UCAction::ResolveLocalLevelDecision(const FActionLocalLevelQuery& InQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EActionLocalLevelDecision::Reject;

	const bool bIsIdle = InQuery.ExecutionState == EExecutionState::Idle;
	const bool bIsActiveAction = InQuery.bIsActiveAction;

	if (bIsIdle && !bIsActiveAction)
	{
		return EActionLocalLevelDecision::Start;
	}

	return EActionLocalLevelDecision::Reject;
}

bool UCAction::Start(const FActionData& InData)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!InData.IsValidMinimal()) return false;
	if (bIsActive) return false;

	ActiveDataKey_Cached = InData.ActionDataKey;
	ActiveData_Cached = InData;
	ActiveMontage_Cached = InData.Montage;

	if (!PlayMontage(InData))
	{
		ClearRuntime();
		return false;
	}

	bIsActive = true;

	RequestFeedback(EActionFeedbackTiming::ActionStart);
	EmitActionEvent(EActionEventType::ActionStarted, ActiveDataKey_Cached.ActionIndex);

	return true;
}

bool UCAction::ApplyChain(const FActionData& InData)
{
	return false;
}

void UCAction::Stop(EActionStopReason InStopReason)
{
	if (!bIsActive) return;

	EActionEventType eventType = EActionEventType::None;
	EActionFinishReason finishReason = EActionFinishReason::None;

	switch (InStopReason)
	{
	case EActionStopReason::Interrupted:
	{
		eventType = EActionEventType::ActionInterrupted;
		finishReason = EActionFinishReason::Interrupted;
		break;
	}
	case EActionStopReason::Cancelled:
	{
		eventType = EActionEventType::ActionCancelled;
		finishReason = EActionFinishReason::Cancelled;
		break;
	}
	default:
		eventType = EActionEventType::ActionIgnored;
		finishReason = EActionFinishReason::Ignored;
		break;
	}

	StopMontage();

	EmitActionEvent(eventType, ActiveDataKey_Cached.ActionIndex);

	ClearRuntime();

	if (IsValid(OwnerActionComp_Injected))
	{
		OwnerActionComp_Injected->HandleApplyActionFinished(this, finishReason);
	}
}

void UCAction::Complete()
{
	if (!bIsActive) return;

	RequestFeedback(EActionFeedbackTiming::ActionComplete);
	EmitActionEvent(EActionEventType::ActionCompleted, ActiveDataKey_Cached.ActionIndex);

	ClearRuntime();

	if (IsValid(OwnerActionComp_Injected))
	{
		OwnerActionComp_Injected->HandleApplyActionFinished(this, EActionFinishReason::Completed);
	}
}

void UCAction::ClearRuntime()
{
	bIsActive = false;

	ActiveDataKey_Cached = FActionDataKey();
	ActiveData_Cached = FActionData();
	ActiveMontage_Cached = nullptr;
}

bool UCAction::PlayMontage(const FActionData& InData)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(InData.Montage)) return false;

	const float duration = OwnerCharacter_Injected->PlayAnimMontage(InData.Montage, InData.PlayRate);

	return duration > 0.0f;
}

void UCAction::StopMontage(float InBlendOutTime)
{
	if (!IsValid(OwnerCharacter_Injected)) return;
	if (!IsValid(ActiveMontage_Cached)) return;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return;

	animInstance->Montage_Stop(InBlendOutTime, ActiveMontage_Cached);
}

void UCAction::HandleNotifyCommand(EActionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EActionNotifyCommand::Complete:
		Complete();
		return;

	case EActionNotifyCommand::PushHitContext:
		PushHitContext();
		return;

	case EActionNotifyCommand::ClearHitContext:
		ClearHitContext();
		return;

	default:
		return;
	}
}

void UCAction::HandleNotifyFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey)
{
	RequestFeedback(InTiming, InTriggerKey);
}

void UCAction::PushHitContext()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->PushContext(BuildActionContext());
}

void UCAction::ClearHitContext()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->ClearContext();
}

void UCAction::RequestFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	if (!IsValid(ActionFeedbackComp_Cached)) return;

	ActionFeedbackComp_Cached->PlayFeedback(BuildFeedbackRequest(InTiming, InTriggerKey));
}

FActionContext UCAction::BuildActionContext() const
{
	FActionContext context;

	context.ActionType = ActiveDataKey_Cached.ActionType;
	context.ActionIndex = ActiveDataKey_Cached.ActionIndex;
	
	return context;
}

FActionFeedbackRequest UCAction::BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FActionFeedbackRequest request;

	request.ActionFeedbackKey.ActionType = ActiveDataKey_Cached.ActionType;
	request.ActionFeedbackKey.ActionIndex = ActiveDataKey_Cached.ActionIndex;
	request.ActionFeedbackTiming = InTiming;
	request.TriggerKey = InTriggerKey;
	
	return request;
}

void UCAction::EmitActionEvent(EActionEventType InEventType, int32 InActionIndex) const
{
	if (!IsValid(OwnerActionComp_Injected)) return;

	const int32 actionIndex = (InActionIndex != INDEX_NONE) ? InActionIndex : ActiveDataKey_Cached.ActionIndex;

	OwnerActionComp_Injected->BroadcastActionEvent(ActiveDataKey_Cached.ActionType, actionIndex, InEventType);
}
