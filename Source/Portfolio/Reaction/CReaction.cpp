#include "Reaction/CReaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"

void UCReaction::Initialize(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	OwnerReactionComp_Injected = InOwnerReactionComponent;
	check(OwnerReactionComp_Injected);

	ReactionFeedbackComp_Cached = OwnerCharacter_Injected->FindComponentByClass<UCReactionFeedbackComponent>();
}

bool UCReaction::IsValidMinimal() const
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(OwnerReactionComp_Injected)) return false;

	const USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return false;

	const UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return false;

	return true;
}

bool UCReaction::Start(const FReactionData& InReactionData)
{
	if (!IsValidMinimal()) return false;
	if (!InReactionData.IsValidMinimal()) return false;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	UAnimInstance* animInstance = meshComp->GetAnimInstance();

	const float playRate = FMath::Max(0.01f, InReactionData.PlayRate);
	const float duration = animInstance->Montage_Play(InReactionData.Montage, playRate);
	if (duration <= 0.f) return false;

	bIsReaction = true;
	ActiveReactionData_Cached = InReactionData;
	ActiveReactionMontage_Cached = InReactionData.Montage;
	LastStopReason_Cached = EReactionStopReason::None;

	RequestFeedback(EReactionFeedbackTiming::ReactionStart);

	const uint32 thisPlaySerial = ++Serial_CurrentPlay;
	CachedSerial_ActivePlay = thisPlaySerial;

	FOnMontageEnded montageEnd;
	montageEnd.BindUObject(this, &UCReaction::OnMontageEnd, thisPlaySerial); // Capture Serial at this time
	animInstance->Montage_SetEndDelegate(montageEnd, ActiveReactionMontage_Cached);

	return true;
}

void UCReaction::Stop(EReactionStopReason InStopReason)
{
	if (!bIsReaction) return;

	if (InStopReason == EReactionStopReason::None)
	{
		LastStopReason_Cached = EReactionStopReason::Aborted;
		FinishAborted();
		return;
	}

	LastStopReason_Cached = InStopReason;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	UAnimInstance* animInstance = IsValid(meshComp) ? meshComp->GetAnimInstance() : nullptr;

	if (!IsValid(animInstance) || !IsValid(ActiveReactionMontage_Cached))
	{
		LastStopReason_Cached = EReactionStopReason::Aborted;
		FinishAborted();
		return;
	}

	// Stop Montage
	animInstance->Montage_Stop(0.1f, ActiveReactionMontage_Cached);

	PrintStopReasonInfo(InStopReason);
	PrintReactionExecutorRuntimeInfo();

	switch (InStopReason)
	{
	case EReactionStopReason::Interrupted:
	{
		FinishInterrupted();
		return;
	}

	case EReactionStopReason::Cancelled:
	{
		FinishCancelled();
		return;
	}

	case EReactionStopReason::Aborted:
	{
		FinishAborted();
		return;
	}

	default:
		LastStopReason_Cached = EReactionStopReason::Aborted;
		FinishAborted();
		return;
	}
}

void UCReaction::FinishCompleted()
{
	if (!bIsReaction) return;

	RequestFeedback(EReactionFeedbackTiming::ReactionCompleted);

	Clear();

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->HandleReactionFinished(this, EReactionFinishReason::Completed);
	}
}

void UCReaction::FinishInterrupted()
{
	if (!bIsReaction) return;

	RequestFeedback(EReactionFeedbackTiming::ReactionInterrupted);

	Clear();

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->HandleReactionFinished(this, EReactionFinishReason::Interrupted);
	}
}

void UCReaction::FinishCancelled()
{
	if (!bIsReaction) return;

	RequestFeedback(EReactionFeedbackTiming::ReactionCancelled);

	Clear();

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->HandleReactionFinished(this, EReactionFinishReason::Cancelled);
	}
}

void UCReaction::FinishAborted()
{
	if (!bIsReaction) return;

	PrintAbortedStopReasonInfo();

	Clear();

	if (IsValid(OwnerReactionComp_Injected))
	{
		OwnerReactionComp_Injected->HandleReactionFinished(this, EReactionFinishReason::Aborted);
	}
}

void UCReaction::Clear()
{
	bIsReaction = false;

	ActiveReactionData_Cached = FReactionData();
	ActiveReactionMontage_Cached = nullptr;
	LastStopReason_Cached = EReactionStopReason::None;

	bInterruptible = false;
	bCancelable = false;
}

void UCReaction::OnReactionControlWindowBegin(EReactionControlWindowType InReactionWindowType)
{
	switch (InReactionWindowType)
	{
	case EReactionControlWindowType::Interruptible:
	{
		SetInterruptible(true);
		break;
	}

	case EReactionControlWindowType::Cancelable:
	{
		SetCancelable(true);
		break;
	}

	case EReactionControlWindowType::ImmuneToReaction:
	{
		SetInterruptible(false);
		SetCancelable(false);
		break;
	}

	default:
		break;
	}
}

void UCReaction::OnReactionControlWindowEnd(EReactionControlWindowType InReactionWindowType)
{
	switch (InReactionWindowType)
	{
	case EReactionControlWindowType::Interruptible:
	{
		SetInterruptible(false);
		break;
	}

	case EReactionControlWindowType::Cancelable:
	{
		SetCancelable(false);
		break;
	}

	case EReactionControlWindowType::ImmuneToReaction:
		break;

	default:
		break;
	}
}

void UCReaction::OnReactionFeedbackWindowBegin(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	RequestFeedback(EReactionFeedbackTiming::WindowBegin, InTriggerKey);
}

void UCReaction::OnReactionFeedbackWindowEnd(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	RequestFeedback(EReactionFeedbackTiming::WindowEnd, InTriggerKey);
}

void UCReaction::OnReactionFeedback(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	RequestFeedback(EReactionFeedbackTiming::Notify, InTriggerKey);
}

bool UCReaction::WantToInterrupt(const FReactionQueryContext& InReactionQueryContext) const
{
	return true;
}

bool UCReaction::WantToCancel(const FReactionQueryContext& InReactionQueryContext) const
{
	return true;
}

bool UCReaction::AllowInterruptionBy(const FReactionQueryContext& InReactionQueryContext) const
{
	return IsInterruptibleNow();
}

bool UCReaction::AllowCancelBy(const FReactionQueryContext& InReactionQueryContext) const
{
	return IsCancelableNow();
}

void UCReaction::RequestFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey) const
{
	if (!IsValid(ReactionFeedbackComp_Cached)) return;
	if (!ActiveReactionData_Cached.IsValidMinimal()) return;

	FReactionFeedbackRequest feedbackRequest = BuildReactionFeedbackRequest(InTiming, InTriggerKey);
	ReactionFeedbackComp_Cached->PlayFeedback(feedbackRequest);
}

FReactionFeedbackRequest UCReaction::BuildReactionFeedbackRequest(EReactionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FReactionFeedbackRequest reactionFeedbackRequest;

	if (!ActiveReactionData_Cached.IsValidMinimal()) return reactionFeedbackRequest;

	reactionFeedbackRequest.ReactionFeedbackKey.ReactionType = ActiveReactionData_Cached.ReactionDataKey.ReactionType;
	reactionFeedbackRequest.ReactionFeedbackKey.ApplyDamageSpecKey = ActiveReactionData_Cached.ReactionDataKey.ApplyDamageSpecKey;
	reactionFeedbackRequest.ReactionFeedbackTiming = InTiming;
	reactionFeedbackRequest.TriggerKey = InTriggerKey;

	return reactionFeedbackRequest;
}

void UCReaction::PrintReactionExecutorRuntimeInfo_Public() const
{
	PrintReactionExecutorRuntimeInfo();
}

void UCReaction::OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial)
{
	if (!CanHandleMontageEnd(InAnimMontage, InSerial)) return;
	if (bInterrupted) return;

	FinishCompleted();
}

bool UCReaction::CanHandleMontageEnd(UAnimMontage* InMontage, uint32 InSerial) const
{
	if (!bIsReaction) return false;
	if (InSerial != CachedSerial_ActivePlay) return false;
	if (InMontage != ActiveReactionMontage_Cached) return false;

	return true;
}

void UCReaction::PrintReactionExecutorRuntimeInfo() const
{
	FLog::Log(TEXT("----- ReactionRuntime Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActiveMontage"), *GetNameSafe(ActiveReactionMontage_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsReaction"), bIsReaction ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bInterruptible"), bInterruptible ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bCancelable"), bCancelable ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_CurrentPlay"), Serial_CurrentPlay));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_ActivePlay"), CachedSerial_ActivePlay));
	FLog::Log(TEXT("---------------------------------"));
}

void UCReaction::PrintStopReasonInfo(EReactionStopReason InStopReason) const
{
	FLog::Log(FString::Printf(TEXT("[Reaction] Stopped. StopReason = %s | ActiveReaction = %s"), *UEnum::GetValueAsString(InStopReason), *GetNameSafe(this)));
}

void UCReaction::PrintAbortedStopReasonInfo() const
{
	FLog::Log(FString::Printf(TEXT("[Reaction] Aborted. StopReason = %s | ActiveReaction = %s"), *UEnum::GetValueAsString(LastStopReason_Cached), *GetNameSafe(this)));
}
