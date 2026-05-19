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

EExecutionDecision UCAction::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EExecutionDecision::Reject;
	if (!InQuery.IncomingPart.IsActionParticipant()) return EExecutionDecision::Reject;

	return EExecutionDecision::Executable;
}

bool UCAction::Start(const FActionData& InData)
{
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

	if (!BindMontageEndDelegate())
	{
		StopMontage();
		ClearRuntime();
		return false;
	}

	bIsActive = true;

	RequestFeedback(EActionFeedbackTiming::Start);
	EmitActionEvent(EActionEventType::ActionStarted, ActiveDataKey_Cached.ActionIndex);

	return true;
}

void UCAction::Stop(EActionStopReason InStopReason)
{
	if (!bIsActive) return;
	if (InStopReason == EActionStopReason::None) return;

	LastStopReason_Cached = InStopReason;

	StopMontage();

	EActionFeedbackTiming feedbackTiming = EActionFeedbackTiming::None;
	EActionEventType eventType = EActionEventType::None;
	EActionFinishReason finishReason = EActionFinishReason::None;

	switch (InStopReason)
	{
	case EActionStopReason::Interrupted:
	{
		feedbackTiming = EActionFeedbackTiming::Interrupt;
		eventType = EActionEventType::ActionInterrupted;
		finishReason = EActionFinishReason::Interrupted;
		break;
	}
	case EActionStopReason::Cancelled:
	{
		feedbackTiming = EActionFeedbackTiming::Cancel;
		eventType = EActionEventType::ActionCancelled;
		finishReason = EActionFinishReason::Cancelled;
		break;
	}
	default:
		eventType = EActionEventType::ActionIgnored;
		finishReason = EActionFinishReason::Ignored;
		break;
	}

	RequestFeedback(feedbackTiming);
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

	RequestFeedback(EActionFeedbackTiming::Complete);
	EmitActionEvent(EActionEventType::ActionCompleted, ActiveDataKey_Cached.ActionIndex);

	ClearRuntime();

	if (IsValid(OwnerActionComp_Injected))
	{
		OwnerActionComp_Injected->HandleApplyActionFinished(this, EActionFinishReason::Completed);
	}
}

bool UCAction::ReserveChain(const FActionData& InData)
{
	// [NOTE] 
	// Specific Actions override this API.
	return false;
}

void UCAction::ConsumeChain()
{
	// [NOTE] 
	// Specific Actions override this API.
}

void UCAction::ClearRuntime()
{
	bIsActive = false;

	ActiveDataKey_Cached = FActionDataKey();
	ActiveData_Cached = FActionData();
	ActiveMontage_Cached = nullptr;
	LastStopReason_Cached = EActionStopReason::None;

	bInterruptible = false;
	bCancelable = false;
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

bool UCAction::BindMontageEndDelegate()
{
	if (!IsValid(OwnerCharacter_Injected)) return false;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return false;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return false;
	if (!IsValid(ActiveMontage_Cached)) return false;

	const uint32 thisPlaySerial = ++Serial_CurrentPlay;
	CachedSerial_ActivePlay = thisPlaySerial;

	FOnMontageEnded montageEnd;
	montageEnd.BindUObject(this, &UCAction::OnMontageEnd, thisPlaySerial);
	animInstance->Montage_SetEndDelegate(montageEnd, ActiveMontage_Cached);

	return true;
}

void UCAction::HandleNotifyCommand(EActionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EActionNotifyCommand::Complete:
		Complete();
		return;

	case EActionNotifyCommand::OpenInterruptWindow:
		SetInterruptible(true);
		return;

	case EActionNotifyCommand::CloseInterruptWindow:
		SetInterruptible(false);
		return;

	case EActionNotifyCommand::OpenCancelWindow:
		SetCancelable(true);
		return;

	case EActionNotifyCommand::CloseCancelWindow:
		SetCancelable(false);
		return;

	case EActionNotifyCommand::PushHitContext:
		PushHitContext();
		return;

	case EActionNotifyCommand::ClearHitContext:
		ClearHitContext();
		return;

	default:
		break;
	}

	HandleSpecificNotifyCommand(InCommand);
}

void UCAction::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
	// [NOTE] 
	// Specific Actions override this API.
}

void UCAction::HandleNotifyFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey)
{
	RequestFeedback(InTiming, InTriggerKey);
}

bool UCAction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	// [NOTE] Base Action Incoming Policy
	// Normal actions do not stop active executions by default.
	// Intentional intervention requires a specific action override.
	return false;
}

bool UCAction::AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	switch (InQuery.StopReason)
	{
	case EExecutionStopReason::Interrupted:
	{
		// [NOTE] Base Action Active Policy
		// Hit/Dead reactions interrupt normal actions by default.
		// Other interrupt types require an explicit interrupt window.
		if (InQuery.IncomingPart.IsReactionParticipant())
		{
			const FReactionExecutionContext& incoming = InQuery.IncomingPart.GetReactionContext();

			if (incoming.ReactionDataKey.ReactionType == EReactionType::Dead) return true;
			if (incoming.ReactionDataKey.ReactionType == EReactionType::Hit) return true;
		}

		// Other interrupt types are controlled by the explicit interrupt window.
		return IsInterruptibleNow();
	}

	case EExecutionStopReason::Cancelled:
	{
		// [NOTE] Base Action Cancel Policy
		// Intentional cancellation requires an explicit cancel window.
		return IsCancelableNow();
	}

	default:
		return false;
	}
}

void UCAction::RequestFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	if (!IsValid(ActionFeedbackComp_Cached)) return;

	FActionFeedbackRequest request = BuildFeedbackRequest(InTiming, InTriggerKey);
	ActionFeedbackComp_Cached->PlayFeedback(request);
}

FActionFeedbackRequest UCAction::BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FActionFeedbackRequest request;

	if (!ActiveDataKey_Cached.IsValidMinimal()) return request;

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

void UCAction::OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial)
{
	if (!CanHandleMontageEnd(InAnimMontage, InSerial)) return;
	if (bInterrupted)
	{
		FLog::Log(TEXT("[Action] Unexpected montage interruption."));
		return;
	}

	Complete();
}

bool UCAction::CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const
{
	if (!bIsActive) return false;
	if (InSerial != CachedSerial_ActivePlay) return false;
	if (InMontage != ActiveMontage_Cached) return false;

	return true;
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

FActionContext UCAction::BuildActionContext() const
{
	FActionContext context;

	context.ActionType = ActiveDataKey_Cached.ActionType;
	context.ActionIndex = ActiveDataKey_Cached.ActionIndex;

	return context;
}
