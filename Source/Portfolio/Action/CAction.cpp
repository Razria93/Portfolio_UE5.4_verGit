#include "Action/CAction.h"

#include "ProjectGlobal.h"

#include "Component/CWeaponComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Core/Debug/FActionComponentDebug.h"

#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

void UCAction::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	WeaponComp_Injected = InReferences.WeaponComponent;
	ActionComp_Injected = InReferences.ActionComponent;
	ActionFeedbackComp_Injected = InReferences.ActionFeedbackComponent;

	ValidateRequiredReferences();
}

bool UCAction::ValidateRequiredReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ WeaponComp_Injected, TEXT("UCWeaponComponent") },
		{ ActionComp_Injected, TEXT("UCActionComponent") },
		{ ActionFeedbackComp_Injected, TEXT("UCActionFeedbackComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

FExecutionDecisionResult UCAction::ResolveExecutionDecision(const FExecutionDecisionQuery& InQuery) const
{
	FExecutionDecisionResult result;

	if (!IsValid(OwnerCharacter_Injected))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!InQuery.IncomingPart.IsActionParticipant())
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	if (!CanResolveIndependentRelationship(InQuery))
	{
		result.Decision = EExecutionDecision::Reject;
		return result;
	}

	result.Decision = EExecutionDecision::Accept;
	result.Relationship = EExecutionRelationship::Independent;
	return result;
}

bool UCAction::TryResolveDeferredConsumeKey(const FExecutionDecisionQuery& InQuery, EDeferredActionConsumeKey& OutConsumeKey) const
{
	OutConsumeKey = EDeferredActionConsumeKey::None;
	return false;
}

bool UCAction::IsIncomingActionType(const FExecutionDecisionQuery& InQuery, EActionType InType) const
{
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();

	return incomingContext.ActionDataKey.ActionType == InType;
}

bool UCAction::IsIncomingActionType(const FExecutionInterventionQuery& InQuery, EActionType InType) const
{
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();

	return incomingContext.ActionDataKey.ActionType == InType;
}

bool UCAction::CanResolveIndependentRelationship(const FExecutionDecisionQuery& InQuery) const
{
	// Idle && No ActivePart: Idle
	return InQuery.Snapshot.IsIdle() && !InQuery.HasActivePart();
}

bool UCAction::CanResolveExclusiveRelationship(const FExecutionDecisionQuery& InQuery) const
{
	// No Idle && Has ActivePart: Active Action OR Active Reaction
	return !InQuery.Snapshot.IsIdle() && InQuery.HasActivePart();
}

bool UCAction::TryResolveIndependentOrExclusiveRelationship(const FExecutionDecisionQuery& InQuery, EExecutionRelationship& OutRelationship) const
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

bool UCAction::Start(const FActionData& InData)
{
	if (!InData.IsValidMinimal())
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, InData, TEXT("Start"), TEXT("InvalidData"));
		return false;
	}

	if (bIsActive)
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, InData, TEXT("Start"), TEXT("AlreadyActive"));
		return false;
	}

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

	const FActionFeedbackRequest feedbackRequest = BuildFeedbackRequest(EActionFeedbackTiming::Start);
	PlayFeedbackRequest(feedbackRequest);
	EmitActionEvent(EActionEventType::ActionStarted, ActiveDataKey_Cached.ActionIndex);
	FActionComponentDebug::RecordActionExecutorStartedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached);
	FActionComponentDebug::PrintActionExecutorRuntimeDebug(OwnerCharacter_Injected, this, ActiveData_Cached, ActiveMontage_Cached, CachedSerial_ActivePlay, TEXT("Start"));

	return true;
}

void UCAction::Interrupt(const FExecutionInterventionDirective& InDirective)
{
	if (!bIsActive)
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("Interrupt"), TEXT("NotActive"));
		return;
	}

	if (!InDirective.IsValidRequest())
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("Interrupt"), TEXT("InvalidDirective"));
		return;
	}

	HandleActionStop(ResolveActionStopReason(InDirective));
}

void UCAction::Stop(EActionStopReason InStopReason)
{
	if (!bIsActive)
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("Stop"), TEXT("NotActive"));
		return;
	}

	if (InStopReason == EActionStopReason::None)
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("Stop"), TEXT("InvalidStopReason"));
		return;
	}

	HandleActionStop(InStopReason);
}

void UCAction::Complete()
{
	if (!bIsActive)
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("Complete"), TEXT("NotActive"));
		return;
	}

	const FActionFeedbackRequest feedbackRequest = BuildFeedbackRequest(EActionFeedbackTiming::Complete);
	const FActionData activeData = ActiveData_Cached;
	const int32 actionIndex = ActiveDataKey_Cached.ActionIndex;

	CleanupRuntimeEffects();
	ClearRuntime();

	PlayFeedbackRequest(feedbackRequest);
	EmitActionEvent(EActionEventType::ActionCompleted, actionIndex);
	FActionComponentDebug::RecordActionExecutorStoppedForAudit(OwnerCharacter_Injected, this, activeData, TEXT("Completed"));

	if (IsValid(ActionComp_Injected))
	{
		ActionComp_Injected->HandleApplyActionFinished(this, EActionFinishReason::Completed);
	}
}

bool UCAction::ReserveChain(const FActionData& InData)
{
	return false;
}

void UCAction::ConsumeChain()
{
}

EActionStopReason UCAction::ResolveActionStopReason(const FExecutionInterventionDirective& InDirective) const
{
	switch (InDirective.StopReason)
	{
	case EExecutionStopReason::Interrupted:
		return EActionStopReason::Interrupted;

	case EExecutionStopReason::Ignored:
	default:
		return EActionStopReason::Ignored;
	}
}

void UCAction::HandleActionStop(EActionStopReason InStopReason)
{
	if (InStopReason == EActionStopReason::None)
	{
		FActionComponentDebug::RecordActionExecutorRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("HandleStop"), TEXT("InvalidStopReason"));
		return;
	}

	LastStopReason_Cached = InStopReason;

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
	default:
		eventType = EActionEventType::ActionIgnored;
		finishReason = EActionFinishReason::Ignored;
		break;
	}

	const FActionFeedbackRequest feedbackRequest = BuildFeedbackRequest(feedbackTiming);
	const FActionData activeData = ActiveData_Cached;
	const int32 actionIndex = ActiveDataKey_Cached.ActionIndex;

	StopMontage();
	CleanupRuntimeEffects();
	ClearRuntime();

	PlayFeedbackRequest(feedbackRequest);
	EmitActionEvent(eventType, actionIndex);
	const FString finishReasonName = UEnum::GetValueAsString(finishReason);
	FActionComponentDebug::RecordActionExecutorStoppedForAudit(OwnerCharacter_Injected, this, activeData, *finishReasonName);

	if (IsValid(ActionComp_Injected))
	{
		ActionComp_Injected->HandleApplyActionFinished(this, finishReason);
	}
}

void UCAction::ClearRuntime()
{
	bIsActive = false;

	ActiveDataKey_Cached = FActionDataKey();
	ActiveData_Cached = FActionData();
	ActiveMontage_Cached = nullptr;
	LastStopReason_Cached = EActionStopReason::None;

	AllowInterventionWindowKeys.Reset();
}

void UCAction::CleanupRuntimeEffects()
{
	if (IsValid(WeaponComp_Injected))
	{
		WeaponComp_Injected->ClearWeaponRuntimeState();
	}

	if (IsValid(ActionFeedbackComp_Injected))
	{
		ActionFeedbackComp_Injected->ClearRuntimeFeedback();
	}
}

bool UCAction::PlayMontage(const FActionData& InData)
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, InData, TEXT("PlayMontage"), TEXT("InvalidOwner"));
		return false;
	}

	if (!IsValid(InData.Montage))
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, InData, TEXT("PlayMontage"), TEXT("InvalidMontage"));
		return false;
	}

	const float duration = OwnerCharacter_Injected->PlayAnimMontage(InData.Montage, InData.PlayRate);
	if (duration <= 0.0f)
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, InData, TEXT("PlayMontage"), TEXT("InvalidDuration"));
		return false;
	}

	FActionComponentDebug::RecordActionMontagePlayedForAudit(OwnerCharacter_Injected, this, InData, duration);

	if (!InData.StartSectionName.IsNone())
	{
		USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
		if (!IsValid(meshComp))
		{
			FActionComponentDebug::RecordActionMontageIgnoredForAudit(OwnerCharacter_Injected, this, InData.Montage, CachedSerial_ActivePlay, CachedSerial_ActivePlay, TEXT("MissingMeshForSectionJump"));
			return true;
		}

		UAnimInstance* animInstance = meshComp->GetAnimInstance();
		if (!IsValid(animInstance))
		{
			FActionComponentDebug::RecordActionMontageIgnoredForAudit(OwnerCharacter_Injected, this, InData.Montage, CachedSerial_ActivePlay, CachedSerial_ActivePlay, TEXT("MissingAnimInstanceForSectionJump"));
			return true;
		}

		animInstance->Montage_JumpToSection(InData.StartSectionName, InData.Montage);
	}

	return true;
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
	if (!IsValid(OwnerCharacter_Injected))
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("BindMontageEnd"), TEXT("InvalidOwner"));
		return false;
	}

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp))
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("BindMontageEnd"), TEXT("InvalidMesh"));
		return false;
	}

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance))
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("BindMontageEnd"), TEXT("InvalidAnimInstance"));
		return false;
	}

	if (!IsValid(ActiveMontage_Cached))
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("BindMontageEnd"), TEXT("InvalidActiveMontage"));
		return false;
	}

	const uint32 thisPlaySerial = ++Serial_CurrentPlay;
	CachedSerial_ActivePlay = thisPlaySerial;

	FOnMontageEnded montageEnd;
	montageEnd.BindUObject(this, &UCAction::OnMontageEnd, thisPlaySerial);
	animInstance->Montage_SetEndDelegate(montageEnd, ActiveMontage_Cached);

	return true;
}

void UCAction::OnMontageEnd(UAnimMontage* InAnimMontage, bool bInterrupted, uint32 InSerial)
{
	if (!CanHandleMontageEnd(InAnimMontage, InSerial))
	{
		FActionComponentDebug::RecordActionMontageIgnoredForAudit(OwnerCharacter_Injected, this, InAnimMontage, InSerial, CachedSerial_ActivePlay, TEXT("StaleMontageEnd"));
		return;
	}

	if (bInterrupted)
	{
		FActionComponentDebug::RecordActionMontageRejectedForAudit(OwnerCharacter_Injected, this, ActiveData_Cached, TEXT("MontageEnd"), TEXT("UnexpectedInterruption"));
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
		break;
	}

	HandleSpecificNotifyCommand(InCommand);
}

void UCAction::HandleSpecificNotifyCommand(EActionNotifyCommand InCommand)
{
}

void UCAction::HandleNotifyFeedback(EActionFeedbackTiming InTiming, FName InTriggerKey)
{
	const FActionFeedbackRequest feedbackRequest = BuildFeedbackRequest(InTiming, InTriggerKey);
	PlayFeedbackRequest(feedbackRequest);
}

bool UCAction::ResolveNotifyCombatSignalCue(FName InCueTag, FActionCombatSignalCueResolution& OutResolution) const
{
	OutResolution = FActionCombatSignalCueResolution();

	if (InCueTag.IsNone()) return false;

	OutResolution.bAccepted = true;
	OutResolution.CueTag = InCueTag;

	return OutResolution.IsValidResolution();
}

void UCAction::PlayFeedbackRequest(const FActionFeedbackRequest& InRequest) const
{
	if (!IsValid(ActionFeedbackComp_Injected)) return;

	ActionFeedbackComp_Injected->PlayFeedback(InRequest);
}

FActionFeedbackRequest UCAction::BuildFeedbackRequest(EActionFeedbackTiming InTiming, FName InTriggerKey) const
{
	FActionFeedbackRequest request;

	if (!ActiveDataKey_Cached.IsValidMinimal()) return request;

	request.ActionFeedbackMatchKey.ActionType = ActiveDataKey_Cached.ActionType;
	request.ActionFeedbackMatchKey.ActionIndex = ActiveDataKey_Cached.ActionIndex;
	request.ActionFeedbackTiming = InTiming;
	request.TriggerKey = InTriggerKey;

	return request;
}

void UCAction::PushHitContext()
{
	if (!IsValid(WeaponComp_Injected))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, this, TEXT("PushHitContext"), NAME_None, TEXT("InvalidWeaponComponent"));
		return;
	}

	WeaponComp_Injected->PushActionDataKey(ActiveDataKey_Cached);
}

void UCAction::ClearHitContext()
{
	if (!IsValid(WeaponComp_Injected))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, this, TEXT("ClearHitContext"), NAME_None, TEXT("InvalidWeaponComponent"));
		return;
	}

	WeaponComp_Injected->ClearContext();
}

void UCAction::OpenAllowInterventionWindow(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	AllowInterventionWindowKeys.Add(InWindowKey);
}

void UCAction::CloseAllowInterventionWindow(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	AllowInterventionWindowKeys.Remove(InWindowKey);
}

bool UCAction::WantIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;
	if (!InQuery.IncomingPart.IsActionParticipant()) return false;

	const FActionExecutionContext& incomingContext = InQuery.IncomingPart.GetActionContext();

	return MatchesWantInterventionRules(incomingContext.ActionData.WantInterventionRules, InQuery.ActivePart);
}

bool UCAction::AllowIntervention(const FExecutionInterventionQuery& InQuery) const
{
	if (!InQuery.IsValidMinimal()) return false;

	return MatchesAllowInterventionRules(ActiveData_Cached.AllowInterventionRules, InQuery.IncomingPart);
}

void UCAction::ResolveObservableOverlayCondition(const FObservableOverlayQuery& InQuery, FObservableOverlayExecutionDecision& OutDecision) const
{
	OutDecision = FObservableOverlayExecutionDecision();

	if (!InQuery.DecisionQuery.IncomingPart.IsActionParticipant())
	{
		// Action only.
		OutDecision.Decision = EExecutionDecision::Reject;
		return;
	}

	// Default Action Case: No overlay cleanup.
	OutDecision.Decision = EExecutionDecision::Accept;
}

bool UCAction::MatchesWantInterventionRules(const TArray<FExecutionInterventionWantRule>& InRules, const FExecutionParticipant& InParticipant) const
{
	for (const FExecutionInterventionWantRule& rule : InRules)
	{
		if (!rule.IsValidMinimal()) continue;
		if (MatchesAnyInterventionFilter(rule.ParticipantFilters, InParticipant)) return true;
	}

	return false;
}

bool UCAction::MatchesAllowInterventionRules(const TArray<FExecutionInterventionAllowRule>& InRules, const FExecutionParticipant& InParticipant) const
{
	for (const FExecutionInterventionAllowRule& rule : InRules)
	{
		if (!rule.IsValidMinimal()) continue;
		if (!IsAllowInterventionRuleTimingSatisfied(rule)) continue;
		if (MatchesAnyInterventionFilter(rule.ParticipantFilters, InParticipant)) return true;
	}

	return false;
}

bool UCAction::IsAllowInterventionRuleTimingSatisfied(const FExecutionInterventionAllowRule& InRule) const
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

bool UCAction::MatchesAnyInterventionFilter(const TArray<FExecutionInterventionParticipantFilter>& InFilters, const FExecutionParticipant& InParticipant) const
{
	for (const FExecutionInterventionParticipantFilter& filter : InFilters)
	{
		if (filter.MatchesParticipant(InParticipant)) return true;
	}

	return false;
}

void UCAction::EmitActionEvent(EActionEventType InEventType, int32 InActionIndex) const
{
	if (!IsValid(ActionComp_Injected)) return;

	const int32 actionIndex = (InActionIndex != INDEX_NONE) ? InActionIndex : ActiveDataKey_Cached.ActionIndex;

	ActionComp_Injected->BroadcastActionEvent(ActiveDataKey_Cached.ActionType, actionIndex, InEventType);
}
