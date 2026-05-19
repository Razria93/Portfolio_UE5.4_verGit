#include "Reaction/CReaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"

void UCReaction::Initialize(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComp)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	OwnerReactionComp_Injected = InOwnerReactionComp;

	if (!IsValid(OwnerCharacter_Injected)) return;

	ReactionFeedbackComp_Cached = OwnerCharacter_Injected->FindComponentByClass<UCReactionFeedbackComponent>();
	check(ReactionFeedbackComp_Cached);
}

EExecutionDecision UCReaction::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	if (!IsValid(OwnerCharacter_Injected)) return EExecutionDecision::Reject;
	if (!InQuery.IncomingPart.IsReactionParticipant()) return EExecutionDecision::Reject;

	return EExecutionDecision::Executable;
}

bool UCReaction::Start(const FReactionData& InData)
{
	if (!InData.IsValidMinimal()) return false;
	if (bIsActive) return false;

	ActiveDataKey_Cached = InData.ReactionDataKey;
	ActiveData_Cached = InData;
	ActiveMontage_Cached = InData.Montage;
	LastStopReason_Cached = EReactionStopReason::None;

	if (!PlayMontage(InData))
	{
		ClearRuntime();
		return false;
	}

	if (!BindMontageEndDelegate())
	{
		StopMontage(0.f);
		ClearRuntime();
		return false;
	}

	bIsActive = true;

	RequestFeedback(EReactionFeedbackTiming::Start);

	return true;
}

void UCReaction::Stop(EReactionStopReason InStopReason)
{
	if (!bIsActive) return;
	if (InStopReason == EReactionStopReason::None) return;

	LastStopReason_Cached = InStopReason;

	StopMontage(0.1f);

	EReactionFeedbackTiming feedbackTiming = EReactionFeedbackTiming::None;
	EReactionFinishReason finishReason = EReactionFinishReason::None;

	switch (InStopReason)
	{
	case EReactionStopReason::Interrupted:
	{
		feedbackTiming = EReactionFeedbackTiming::Interrupt;
		finishReason = EReactionFinishReason::Interrupted;
		break;
	}
	case EReactionStopReason::Cancelled:
	{
		feedbackTiming = EReactionFeedbackTiming::Cancel;
		finishReason = EReactionFinishReason::Cancelled;
		break;
	}
	default:
		finishReason = EReactionFinishReason::Ignored;
		break;
	}

	RequestFeedback(feedbackTiming);

	ClearRuntime();

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->HandleApplyReactionFinished(this, finishReason);
	}
}

void UCReaction::Complete()
{
	if (!bIsActive) return;

	RequestFeedback(EReactionFeedbackTiming::Complete);

	ClearRuntime();

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->HandleApplyReactionFinished(this, EReactionFinishReason::Completed);
	}
}

void UCReaction::ClearRuntime()
{
	bIsActive = false;

	ActiveDataKey_Cached = FReactionDataKey();
	ActiveData_Cached = FReactionData();
	ActiveMontage_Cached = nullptr;
	LastStopReason_Cached = EReactionStopReason::None;

	bWantInterrupt = false;
	bWantCancel = false;
	bAllowInterrupt = false;
	bAllowCancel = false;
}

bool UCReaction::PlayMontage(const FReactionData& InData)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(InData.Montage)) return false;

	const float duration = OwnerCharacter_Injected->PlayAnimMontage(InData.Montage, InData.PlayRate);

	return duration > 0.0f;
}

void UCReaction::StopMontage(float InBlendOutTime)
{
	if (!IsValid(OwnerCharacter_Injected)) return;
	if (!IsValid(ActiveMontage_Cached)) return;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return;

	animInstance->Montage_Stop(InBlendOutTime, ActiveMontage_Cached);
}

bool UCReaction::BindMontageEndDelegate()
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
	montageEnd.BindUObject(this, &UCReaction::OnMontageEnd, thisPlaySerial);
	animInstance->Montage_SetEndDelegate(montageEnd, ActiveMontage_Cached);

	return true;
}

void UCReaction::HandleNotifyCommand(EReactionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EReactionNotifyCommand::Complete:
		Complete();
		return;

	case EReactionNotifyCommand::OpenWantInterruptWindow:
		SetWantInterrupt(true);
		return;

	case EReactionNotifyCommand::CloseWantInterruptWindow:
		SetWantInterrupt(false);
		return;

	case EReactionNotifyCommand::OpenWantCancelWindow:
		SetWantCancel(true);
		return;

	case EReactionNotifyCommand::CloseWantCancelWindow:
		SetWantCancel(false);
		return;

	case EReactionNotifyCommand::OpenAllowInterruptWindow:
		SetAllowInterrupt(true);
		return;

	case EReactionNotifyCommand::CloseAllowInterruptWindow:
		SetAllowInterrupt(false);
		return;

	case EReactionNotifyCommand::OpenAllowCancelWindow:
		SetAllowCancel(true);
		return;

	case EReactionNotifyCommand::CloseAllowCancelWindow:
		SetAllowCancel(false);
		return;

	default:
		break;
	}

	HandleSpecificNotifyCommand(InCommand);
}

void UCReaction::HandleSpecificNotifyCommand(EReactionNotifyCommand InCommand)
{
	// [NOTE] 
	// Specific Reactions override this API.
}

void UCReaction::HandleNotifyFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey)
{
	RequestFeedback(InTiming, InTriggerKey);
}

bool UCReaction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	switch (InQuery.StopReason)
	{
	case EExecutionStopReason::Interrupted:
	{
		// [NOTE] Base Reaction Incoming Policy
		// Reactions request interrupt only while the want-interrupt window is open.
		// Force reactions such as DeadReaction should be handled by orchestration or subclass override.
		return IsWantInterruptNow();
	}

	case EExecutionStopReason::Cancelled:
	{
		// [NOTE] Base Reaction Incoming Cancel Policy
		// Reactions request cancel only while the want-cancel window is open.
		return IsWantCancelNow();
	}

	default:
		return false;
	}
}

bool UCReaction::AllowInterventionBy(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	switch (InQuery.StopReason)
	{

	case EExecutionStopReason::Interrupted:
	{
		// [NOTE] Base Reaction Active Policy
		// Active reactions allow interrupts only while the allow-interrupt window is open.
		return IsAllowInterruptNow();
	}

	case EExecutionStopReason::Cancelled:
	{
		// [NOTE] Base Reaction Active Cancel Policy
		// Active reactions allow intentional cancel only while the allow-cancel window is open.
		return IsAllowCancelNow();
	}

	default:
		return false;
	}
}

void UCReaction::RequestFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey) const
{
	if (!IsValid(ReactionFeedbackComp_Cached)) return;

	FReactionFeedbackRequest request = BuildFeedbackRequest(InTiming, InTriggerKey);
	ReactionFeedbackComp_Cached->PlayFeedback(request);
}

FReactionFeedbackRequest UCReaction::BuildFeedbackRequest(EReactionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FReactionFeedbackRequest request;

	if (!ActiveDataKey_Cached.IsValidMinimal()) return request;

	request.ReactionFeedbackKey.ReactionType = ActiveDataKey_Cached.ReactionType;
	request.ReactionFeedbackKey.ApplyDamageSpecKey = ActiveDataKey_Cached.ApplyDamageSpecKey;
	request.ReactionFeedbackTiming = InTiming;
	request.TriggerKey = InTriggerKey;

	return request;
}

void UCReaction::OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial)
{
	if (!CanHandleMontageEnd(InAnimMontage, InSerial)) return;
	if (bInterrupted)
	{
		FLog::Log(TEXT("[Reaction] Unexpected montage interruption."));
		return;
	}

	Complete();
}

bool UCReaction::CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const
{
	if (!bIsActive) return false;
	if (InSerial != CachedSerial_ActivePlay) return false;
	if (InMontage != ActiveMontage_Cached) return false;

	return true;
}

void UCReaction::PrintReactionExecutorRuntimeInfo_Public() const
{
	PrintReactionExecutorRuntimeInfo();
}

void UCReaction::PrintReactionExecutorRuntimeInfo() const
{
	FLog::Log(TEXT("----- ReactionRuntime Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActiveMontage"), *GetNameSafe(ActiveMontage_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsActive"), bIsActive ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bWantInterrupt"), bWantInterrupt ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bWantCancel"), bWantCancel ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bAllowInterrupt"), bAllowInterrupt ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bAllowCancel"), bAllowCancel ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_CurrentPlay"), Serial_CurrentPlay));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_ActivePlay"), CachedSerial_ActivePlay));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReaction::PrintStopReasonInfo(EReactionStopReason InStopReason) const
{
	FLog::Log(FString::Printf(TEXT("[Reaction] Stopped. StopReason = %s | ActiveReaction = %s"), *UEnum::GetValueAsString(InStopReason), *GetNameSafe(this)));
}

void UCReaction::PrintIgnoredStopReasonInfo() const
{
	FLog::Log(FString::Printf(TEXT("[Reaction] Ignored. StopReason = %s | ActiveReaction = %s"), *UEnum::GetValueAsString(LastStopReason_Cached), *GetNameSafe(this)));
}
