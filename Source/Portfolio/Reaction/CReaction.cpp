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

bool UCReaction::CanResolveExclusiveRelationship(const FExecutionDecisionQuery& InQuery) const
{
	// No Idle && Has ActivePart: Active Action OR Active Reaction
	return !InQuery.Snapshot.IsIdle() && InQuery.HasActivePart();
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

	if (CanResolveExclusiveRelationship(InQuery))
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

	AllowInterventionWindowKeys.Reset();
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

void UCReaction::OpenAllowInterventionWindow(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	AllowInterventionWindowKeys.Add(InWindowKey);
}

void UCReaction::CloseAllowInterventionWindow(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	AllowInterventionWindowKeys.Remove(InWindowKey);
}

bool UCReaction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsReactionParticipant()) return false;

	const FReactionExecutionContext& incomingContext = InQuery.IncomingPart.GetReactionContext();

	return MatchesWantInterventionRules(incomingContext.ReactionData.WantInterventionRules, InQuery.ActivePart);
}

bool UCReaction::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	return MatchesAllowInterventionRules(ActiveData_Cached.AllowInterventionRules, InQuery.IncomingPart);
}

bool UCReaction::MatchesWantInterventionRules(const TArray<FExecutionInterventionWantRule>& InRules, const FExecutionParticipant& InParticipant) const
{
	for (const FExecutionInterventionWantRule& rule : InRules)
	{
		if (!rule.IsValidMinimal()) continue;
		if (MatchesAnyInterventionFilter(rule.ParticipantFilters, InParticipant)) return true;
	}

	return false;
}

bool UCReaction::MatchesAllowInterventionRules(const TArray<FExecutionInterventionAllowRule>& InRules, const FExecutionParticipant& InParticipant) const
{
	for (const FExecutionInterventionAllowRule& rule : InRules)
	{
		if (!rule.IsValidMinimal()) continue;
		if (!IsAllowInterventionRuleTimingSatisfied(rule)) continue;
		if (MatchesAnyInterventionFilter(rule.ParticipantFilters, InParticipant)) return true;
	}

	return false;
}

bool UCReaction::IsAllowInterventionRuleTimingSatisfied(const FExecutionInterventionAllowRule& InRule) const
{
	switch (InRule.Timing)
	{
	case EExecutionInterventionTiming::Always:
		return true;

	case EExecutionInterventionTiming::Window:
		return !InRule.WindowKey.IsNone() && AllowInterventionWindowKeys.Contains(InRule.WindowKey);

	default:
		return false;
	}
}

bool UCReaction::MatchesAnyInterventionFilter(const TArray<FExecutionInterventionParticipantFilter>& InFilters, const FExecutionParticipant& InParticipant) const
{
	// PrintExecutionParticipant(InParticipant);

	// Match the actual query participant against counterpart filters opened by notify windows.
	for (const FExecutionInterventionParticipantFilter& filter : InFilters)
	{
		// PrintExecutionInterventionParticipantFilter(filter);

		if (filter.MatchesParticipant(InParticipant)) return true;
	}

	FLog::Log(TEXT("[UCReaction::MatchesAnyInterventionFilter] Match Failed."));
	return false;
}

void UCReaction::PrintReactionExecutorRuntimeInfo_Public() const
{
	PrintReactionExecutorRuntimeInfo();
}

void UCReaction::PrintExecutionParticipant(const FExecutionParticipant& InParticipant)
{
	FLog::Log(TEXT("======== Participant ID ========="));

	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsValid"), InParticipant.bIsValid ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Domain"), *UEnum::GetValueAsString(InParticipant.ParticipantDomain)));

	if (InParticipant.IsActionParticipant())
	{
		const FActionExecutionContext& context = InParticipant.GetActionContext();

		FLog::Log(TEXT("--------- Action Context --------"));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(context.ActionDataKey.ActionType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("ActionIndex"), context.ActionDataKey.ActionIndex));
		FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Priority"), context.ActionData.Priority));
	}
	else if (InParticipant.IsReactionParticipant())
	{
		const FReactionExecutionContext& context = InParticipant.GetReactionContext();
		const FApplyDamageSpecKey& specKey = context.ReactionDataKey.ApplyDamageSpecKey;

		FLog::Log(TEXT("-------- Reaction Context -------"));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(context.ReactionDataKey.ReactionType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("SpecKey|WeaponType"), *UEnum::GetValueAsString(specKey.WeaponType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("SpecKey|ActionType"), *UEnum::GetValueAsString(specKey.ActionType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("SpecKey|ActionIndex"), specKey.ActionIndex));
		FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("Priority"), context.ReactionData.Priority));
	}
	else
	{
		FLog::Log(TEXT("[ExecutionParticipant] Empty or invalid participant context."));
	}

	FLog::Log(TEXT("================================="));
}

void UCReaction::PrintExecutionInterventionParticipantFilter(const FExecutionInterventionParticipantFilter& InFilter)
{
	FLog::Log(TEXT("===== Participant Filter ID ====="));
	
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("IsValid"), InFilter.IsValidMinimal() ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Domain"), *UEnum::GetValueAsString(InFilter.Domain)));

	switch (InFilter.Domain)
	{
	case EExecutionDomain::Action:
	{
		const FString indexText = (InFilter.Index == INDEX_NONE) ? TEXT("ANY") : FString::FromInt(InFilter.Index);
		
		FLog::Log(TEXT("--------- Action Filter ---------"));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActionType"), *UEnum::GetValueAsString(InFilter.ActionType)));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("Index"), *indexText));
		break;
	}

	case EExecutionDomain::Reaction:
	{
		FLog::Log(TEXT("-------- Reaction Filter --------"));
		FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ReactionType"), *UEnum::GetValueAsString(InFilter.ReactionType)));
		break;
	}

	default:
	{
		FLog::Log(TEXT("[InterventionFilter] Invalid domain."));
		break;
	}
	}

	FLog::Log(TEXT("================================="));
}

void UCReaction::PrintReactionExecutorRuntimeInfo() const
{
	FLog::Log(TEXT("----- ReactionRuntime Info ------"));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActiveMontage"), *GetNameSafe(ActiveMontage_Cached)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("bIsActive"), bIsActive ? TEXT("true") : TEXT("false")));
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("AllowWindowKeyCount"), AllowInterventionWindowKeys.Num()));
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
