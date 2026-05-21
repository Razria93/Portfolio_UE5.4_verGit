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

FExecutionDecisionResult UCReaction::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!InQuery.IncomingPart.IsReactionParticipant())
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	EExecutionRelationship relationship = EExecutionRelationship::None;

	if (!TryResolveIndependentOrExclusiveRelationship(InQuery, relationship))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = relationship;
	return result;
}

bool UCReaction::IsIncomingReactionType(const FExecutionDecisionQuery& InQuery, EReactionType InType) const
{
	if (!InQuery.IncomingPart.IsReactionParticipant()) return false;

	const FReactionExecutionContext& incomingContext = InQuery.IncomingPart.GetReactionContext();

	return incomingContext.ReactionDataKey.ReactionType == InType;
}

bool UCReaction::IsIncomingReactionType(const FExecutionInterventionQuery& InQuery, EReactionType InType) const
{
	if (!InQuery.IncomingPart.IsReactionParticipant()) return false;

	const FReactionExecutionContext& incomingContext = InQuery.IncomingPart.GetReactionContext();

	return incomingContext.ReactionDataKey.ReactionType == InType;
}

bool UCReaction::CanResolveIndependentRelationship(const FExecutionDecisionQuery& InQuery) const
{
	// Idle && No ActivePart: Idle
	return InQuery.Snapshot.IsIdle() && !InQuery.HasActivePart();
}

bool UCReaction::TryResolveIndependentOrExclusiveRelationship(const FExecutionDecisionQuery& InQuery, EExecutionRelationship& OutRelationship) const
{
	OutRelationship = EExecutionRelationship::None;

	// Idle && No ActivePart: Idle
	if (CanResolveIndependentRelationship(InQuery))
	{
		OutRelationship = EExecutionRelationship::Independent;
		return true;
	}

	// No Idle && Has ActivePart: Active Action OR Active Reaction
	if (!InQuery.Snapshot.IsIdle() && InQuery.HasActivePart())
	{
		OutRelationship = EExecutionRelationship::Exclusive;
		return true;
	}

	return false;
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

	WantCancelFilters.Reset();
	WantInterruptFilters.Reset();
	AllowCancelFilters.Reset();
	AllowInterruptFilters.Reset();
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

void UCReaction::HandleNotifyCommand(EReactionNotifyCommand InCommand)
{
	switch (InCommand)
	{
	case EReactionNotifyCommand::Complete:
		Complete();
		return;

	default:
		break;
	}

	HandleSpecificNotifyCommand(InCommand);
}

void UCReaction::HandleSpecificNotifyCommand(EReactionNotifyCommand InCommand)
{
	// Specific Reactions override this API.
}

void UCReaction::HandleNotifyFeedback(EReactionFeedbackTiming InTiming, FName InTriggerKey)
{
	RequestFeedback(InTiming, InTriggerKey);
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

void UCReaction::OpenInterventionWindow(
	const FExecutionInterventionParticipantFilter& InOwnerFilter, EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole, const TArray<FExecutionInterventionParticipantFilter>& InCounterpartFilters)
{
	if (!MatchesInterventionOwner(InOwnerFilter)) return;

	TArray<FExecutionInterventionParticipantFilter>* container = GetInterventionFilterContainer(InStopReason, InWindowRole);
	if (!container) return;

	for (const FExecutionInterventionParticipantFilter& filter : InCounterpartFilters)
	{
		if (!filter.IsValidMinimal()) continue;

		container->Add(filter);
	}
}

void UCReaction::CloseInterventionWindow(
	const FExecutionInterventionParticipantFilter& InOwnerFilter, EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole, const TArray<FExecutionInterventionParticipantFilter>& InCounterpartFilters)
{
	if (!MatchesInterventionOwner(InOwnerFilter)) return;

	TArray<FExecutionInterventionParticipantFilter>* container = GetInterventionFilterContainer(InStopReason, InWindowRole);
	if (!container) return;

	for (const FExecutionInterventionParticipantFilter& filter : InCounterpartFilters)
	{
		container->RemoveSingle(filter);
	}
}

bool UCReaction::MatchesWantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	const TArray<FExecutionInterventionParticipantFilter>* filters = GetInterventionFilterContainer(InQuery.StopReason, EExecutionInterventionWindowRole::Want);

	return filters && MatchesAnyInterventionFilter(*filters, InQuery.ActivePart);
}

bool UCReaction::MatchesAllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	const TArray<FExecutionInterventionParticipantFilter>* filters = GetInterventionFilterContainer(InQuery.StopReason, EExecutionInterventionWindowRole::Allow);

	return filters && MatchesAnyInterventionFilter(*filters, InQuery.IncomingPart);
}

bool UCReaction::MatchesInterventionOwner(const FExecutionInterventionParticipantFilter& InOwnerFilter) const
{
	// Check whether this active executor is the owner targeted by the notify window.
	if (!bIsActive) return false;

	return InOwnerFilter.MatchesReaction(ActiveDataKey_Cached.ReactionType);
}

bool UCReaction::MatchesAnyInterventionFilter(const TArray<FExecutionInterventionParticipantFilter>& InFilters, const FExecutionParticipant& InParticipant) const
{
	// Match the actual query participant against counterpart filters opened by notify windows.
	for (const FExecutionInterventionParticipantFilter& filter : InFilters)
	{
		if (filter.MatchesParticipant(InParticipant)) return true;
	}

	return false;
}

TArray<FExecutionInterventionParticipantFilter>* UCReaction::GetInterventionFilterContainer(EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole)
{
	switch (InWindowRole)
	{
	case EExecutionInterventionWindowRole::Want:
		if (InStopReason == EExecutionStopReason::Cancelled) return &WantCancelFilters;
		if (InStopReason == EExecutionStopReason::Interrupted) return &WantInterruptFilters;
		return nullptr;

	case EExecutionInterventionWindowRole::Allow:
		if (InStopReason == EExecutionStopReason::Cancelled) return &AllowCancelFilters;
		if (InStopReason == EExecutionStopReason::Interrupted) return &AllowInterruptFilters;
		return nullptr;

	default:
		return nullptr;
	}
}

const TArray<FExecutionInterventionParticipantFilter>* UCReaction::GetInterventionFilterContainer(EExecutionStopReason InStopReason, EExecutionInterventionWindowRole InWindowRole) const
{
	switch (InWindowRole)
	{
	case EExecutionInterventionWindowRole::Want:
		if (InStopReason == EExecutionStopReason::Cancelled) return &WantCancelFilters;
		if (InStopReason == EExecutionStopReason::Interrupted) return &WantInterruptFilters;
		return nullptr;

	case EExecutionInterventionWindowRole::Allow:
		if (InStopReason == EExecutionStopReason::Cancelled) return &AllowCancelFilters;
		if (InStopReason == EExecutionStopReason::Interrupted) return &AllowInterruptFilters;
		return nullptr;

	default:
		return nullptr;
	}
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
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("WantCancelCount"), WantCancelFilters.Num()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("WantInterruptCount"), WantInterruptFilters.Num()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("AllowCancelCount"), AllowCancelFilters.Num()));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("AllowInterruptCount"), AllowInterruptFilters.Num()));
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
