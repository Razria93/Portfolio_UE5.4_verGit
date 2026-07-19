#include "Reaction/CReaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"

void UCReaction::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	ReactionComp_Injected = InReferences.ReactionComponent;
	ReactionFeedbackComp_Injected = InReferences.ReactionFeedbackComponent;

	ValidateRequiredReferences();
}

bool UCReaction::ValidateRequiredReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ ReactionComp_Injected, TEXT("UCReactionComponent") },
		{ ReactionFeedbackComp_Injected, TEXT("UCReactionFeedbackComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
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

	const FReactionFeedbackRequest feedbackRequest = BuildFeedbackRequest(EReactionFeedbackTiming::Start);
	PlayFeedbackRequest(feedbackRequest);

	return true;
}

void UCReaction::Interrupt(const FExecutionInterventionDirective& InDirective)
{
	if (!bIsActive) return;
	if (!InDirective.IsValidRequest()) return;

	HandleReactionStop(ResolveReactionStopReason(InDirective));
}

void UCReaction::Stop(EReactionStopReason InStopReason)
{
	if (!bIsActive) return;
	if (InStopReason == EReactionStopReason::None) return;

	HandleReactionStop(InStopReason);
}

void UCReaction::Complete()
{
	if (!bIsActive) return;

	const FReactionFeedbackRequest feedbackRequest = BuildFeedbackRequest(EReactionFeedbackTiming::Complete);

	CleanupRuntimeEffects();
	ClearRuntime();

	PlayFeedbackRequest(feedbackRequest);

	if (IsValid(ReactionComp_Injected))
	{
		ReactionComp_Injected->HandleApplyReactionFinished(this, EReactionFinishReason::Completed);
	}
}

EReactionStopReason UCReaction::ResolveReactionStopReason(const FExecutionInterventionDirective& InDirective) const
{
	switch (InDirective.StopReason)
	{
	case EExecutionStopReason::Interrupted:
		return EReactionStopReason::Interrupted;

	case EExecutionStopReason::Ignored:
	default:
		return EReactionStopReason::Ignored;
	}
}

void UCReaction::HandleReactionStop(EReactionStopReason InStopReason)
{
	if (InStopReason == EReactionStopReason::None) return;

	LastStopReason_Cached = InStopReason;

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

	const FReactionFeedbackRequest feedbackRequest = BuildFeedbackRequest(feedbackTiming);

	StopMontage(0.1f);
	CleanupRuntimeEffects();
	ClearRuntime();

	PlayFeedbackRequest(feedbackRequest);

	if (IsValid(ReactionComp_Injected))
	{
		ReactionComp_Injected->HandleApplyReactionFinished(this, finishReason);
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

void UCReaction::CleanupRuntimeEffects()
{
	if (IsValid(ReactionFeedbackComp_Injected))
	{
		ReactionFeedbackComp_Injected->ClearRuntimeFeedback();
	}
}

bool UCReaction::PlayMontage(const FReactionData& InData)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(InData.Montage)) return false;

	const float duration = OwnerCharacter_Injected->PlayAnimMontage(InData.Montage, InData.PlayRate);
	if (duration <= 0.0f) return false;

	if (!InData.StartSectionName.IsNone())
	{
		USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
		if (!IsValid(meshComp)) return true;

		UAnimInstance* animInstance = meshComp->GetAnimInstance();
		if (!IsValid(animInstance)) return true;

		animInstance->Montage_JumpToSection(InData.StartSectionName, InData.Montage);
	}

	return true;
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
		FLog::Log(FString::Printf(
			TEXT("[Reaction] Unexpected montage interruption. Reaction=%s | Montage=%s | Serial=%u"),
			*GetNameSafe(this),
			*GetNameSafe(InAnimMontage),
			InSerial));
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
	const FReactionFeedbackRequest feedbackRequest = BuildFeedbackRequest(InTiming, InTriggerKey);
	PlayFeedbackRequest(feedbackRequest);
}

void UCReaction::PlayFeedbackRequest(const FReactionFeedbackRequest& InRequest) const
{
	if (!IsValid(ReactionFeedbackComp_Injected)) return;

	ReactionFeedbackComp_Injected->PlayFeedback(InRequest);
}

FReactionFeedbackRequest UCReaction::BuildFeedbackRequest(EReactionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FReactionFeedbackRequest request;

	if (!ActiveDataKey_Cached.IsValidMinimal()) return request;

	request.ReactionFeedbackKey.ReactionType = ActiveDataKey_Cached.ReactionType;
	request.ReactionFeedbackKey.DamageSpecKey = ActiveDataKey_Cached.DamageSpecKey;
	request.ReactionFeedbackTiming = InTiming;
	request.TriggerKey = InTriggerKey;

	return request;
}

void UCReaction::RequestConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey) const
{
	if (!IsValid(ReactionComp_Injected)) return;

	ReactionComp_Injected->RequestConsumeDeferredAction(InConsumeKey);
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

void UCReaction::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	if (!InQuery.DecisionQuery.IncomingPart.IsReactionParticipant())
	{
		// Reaction only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	// Default Reaction Case: No overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
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
	// Match the actual query participant against counterpart filters opened by notify windows.
	for (const FExecutionInterventionParticipantFilter& filter : InFilters)
	{
		if (filter.MatchesParticipant(InParticipant)) return true;
	}

	return false;
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
	FLog::Log(FString::Printf(TEXT("%-20s: %d"), TEXT("AllowWindowKeyCount"), AllowInterventionWindowKeys.Num()));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_CurrentPlay"), Serial_CurrentPlay));
	FLog::Log(FString::Printf(TEXT("%-20s: %u"), TEXT("Serial_ActivePlay"), CachedSerial_ActivePlay));
	FLog::Log(TEXT("---------------------------------"));
}
