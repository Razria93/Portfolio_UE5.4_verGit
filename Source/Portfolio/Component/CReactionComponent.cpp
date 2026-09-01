#include "Component/CReactionComponent.h"

#include "ProjectGlobal.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Reaction/CReaction.h"
#include "Core/Debug/FReactionComponentDebug.h"
#include "Type/CWeaponTypes.h"
#include "Type/CActionTypes.h"
#include "Type/CReactionTypes.h"
#include "Type/CReactionDataTypes.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "Type/CCombatDamageTypes.h"
#include "Type/CObservableOverlayTypes.h"
#include "Type/CExecutionTypes.h"

#include "GameFramework/Character.h"

// Construction

UCReactionComponent::UCReactionComponent()
{
}

void UCReactionComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	MovementComp_Injected = InReferences.MovementComponent;
	StateComp_Injected = InReferences.StateComponent;
	HealthComp_Injected = InReferences.HealthComponent;
	ObservableOverlayComp_Injected = InReferences.ObservableOverlayComponent;
	ActionComp_Injected = InReferences.ActionComponent;
	ReactionFeedbackComp_Injected = InReferences.ReactionFeedbackComponent;

	ValidateRequiredComponentReferences();
}

// Lifecycle

void UCReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeReactionRuntime();
}

void UCReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeReactionRuntime();

	Super::EndPlay(EndPlayReason);
}

// Query

bool UCReactionComponent::IsActive() const
{
	return ActiveReactionType != EReactionType::None
		&& ActiveReactionType != EReactionType::Idle
		&& ActiveReactionType != EReactionType::All
		&& ActiveReactionType != EReactionType::Max;
}

EReactionType UCReactionComponent::GetActiveReactionType() const
{
	return ActiveReactionType;
}

bool UCReactionComponent::GetActiveReactionData(FReactionData& OutData) const
{
	OutData = FReactionData();

	if (!IsActive()) return false;
	if (!ActiveReactionData.IsValidMinimal()) return false;

	OutData = ActiveReactionData;
	return true;
}

UCReaction* UCReactionComponent::GetActiveReactionExecutor() const
{
	if (!IsActive()) return nullptr;
	if (!IsValid(ActiveReactionExecutor)) return nullptr;

	return ActiveReactionExecutor;
}

bool UCReactionComponent::GetActiveReactionContext(FReactionExecutionContext& OutContext) const
{
	OutContext = FReactionExecutionContext();
	if (!IsActive() || !ActiveReactionContext.IsValidMinimal()) return false;

	OutContext = ActiveReactionContext;
	return true;
}

// Data Resolve

bool UCReactionComponent::ResolveReactionData(const FReactionDataKey& InDataKey, FReactionData& OutData)
{
	OutData = FReactionData();

	if (!InDataKey.IsValidMinimal())
	{
		FReactionComponentDebug::RecordReactionDataResolveFailedForAudit(OwnerCharacter_Injected, InDataKey, TEXT("InvalidDataKey"));
		return false;
	}

	switch (InDataKey.MatchMode)
	{
	case EReactionDataMatchMode::Global:
		return ResolveGlobalReactionData(InDataKey, OutData);

	case EReactionDataMatchMode::DamageSpec:
		return ResolveDamageSpecReactionData(InDataKey, OutData);

	default:
		FReactionComponentDebug::RecordReactionDataResolveFailedForAudit(OwnerCharacter_Injected, InDataKey, TEXT("UnsupportedMatchMode"));
		return false;
	}
}

UCReaction* UCReactionComponent::ResolveReactionExecutor(const FReactionData& InData)
{
	// Preferred: reuse cached reaction executor.
	UCReaction* found = FindReactionExecutor(InData.ReactionExecutorKey.Get());
	if (IsValid(found)) return found;

	// Fallback: create and cache reaction executor.
	UCReaction* add = AddReactionExecutor(InData.ReactionExecutorKey);
	if (IsValid(add)) return add;

	FReactionComponentDebug::RecordReactionExecutorResolveFailedForAudit(OwnerCharacter_Injected, InData, TEXT("MissingExecutor"));
	return nullptr;
}

// Execution Entry

bool UCReactionComponent::ApplyReactionDecision(const FReactionExecutionResult& InResult)
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("InvalidOwner"));
		return false;
	}

	if (!InResult.IsAcceptedDecision())
	{
		FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("RejectedDecision"));
		return false;
	}

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
	{
		if (!ApplyOverlayHandlings(InResult.OverlayHandlings))
		{
			FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("Start"), TEXT("OverlayHandlingFailed"));
			return false;
		}

		bool bStarted = StartReaction(InResult.ResolvedContext);
		if (bStarted)
		{
			FReactionComponentDebug::RecordReactionDecisionAppliedForAudit(OwnerCharacter_Injected, InResult, TEXT("Start"));
			FReactionComponentDebug::PrintReactionExecutionContextDebug(OwnerCharacter_Injected, InResult.ResolvedContext, TEXT("Start"));
		}
		else
		{
			FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("Start"), TEXT("StartFailed"));
		}
		return bStarted;
	}

	case EExecutionApplyMode::Reserve:
	{
		// Reaction does not support reserved execution.
		FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("Reserve"), TEXT("UnsupportedReserve"));
		return false;
	}

	case EExecutionApplyMode::Intervene:
	{
		if (!ApplyExecutionInterventionDirective(InResult.InterventionDirective))
		{
			FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("Intervene"), TEXT("InterventionFailed"));
			return false;
		}
		if (!ApplyOverlayHandlings(InResult.OverlayHandlings))
		{
			FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("Intervene"), TEXT("OverlayHandlingFailed"));
			return false;
		}

		bool bStarted = StartReaction(InResult.ResolvedContext);
		if (bStarted)
		{
			FReactionComponentDebug::RecordReactionDecisionAppliedForAudit(OwnerCharacter_Injected, InResult, TEXT("Intervene"));
			FReactionComponentDebug::PrintReactionExecutionContextDebug(OwnerCharacter_Injected, InResult.ResolvedContext, TEXT("Intervene"));
		}
		else
		{
			FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("Intervene"), TEXT("StartFailed"));
		}
		return bStarted;
	}
	
	default:
		FReactionComponentDebug::RecordReactionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("UnsupportedApplyMode"));
		return false;
	}
}

bool UCReactionComponent::RequestInterruptActiveReaction(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsValidRequest())
	{
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, FReactionExecutionContext(), TEXT("RequestInterrupt"), TEXT("InvalidDirective"));
		return false;
	}
	if (InDirective.TargetDomain != EExecutionDomain::Reaction)
	{
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, FReactionExecutionContext(), TEXT("RequestInterrupt"), TEXT("WrongTargetDomain"));
		return false;
	}

	return InterruptActiveReaction(InDirective);
}

// Execution Result Hooks

void UCReactionComponent::HandleApplyReactionFinished(const UCReaction* InReaction, EReactionFinishReason InFinishReason)
{
	if (!IsActive())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, InReaction, TEXT("ApplyFinished"), NAME_None, TEXT("NotActive"));
		return;
	}
	if (!IsValid(InReaction))
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, InReaction, TEXT("ApplyFinished"), NAME_None, TEXT("InvalidReaction"));
		return;
	}
	if (InReaction != GetActiveReactionExecutor())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, InReaction, TEXT("ApplyFinished"), NAME_None, TEXT("StaleReaction"));
		return;
	}

	EndActiveReaction(InFinishReason);
}

// Cross-System Dispatch

void UCReactionComponent::RequestConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey)
{
	if (!IsValid(ActionComp_Injected))
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, TEXT("ConsumeDeferredAction"), NAME_None, TEXT("InvalidActionComponent"));
		return;
	}

	ActionComp_Injected->ConsumeDeferredAction(InConsumeKey);
}

// Notify Routing

void UCReactionComponent::HandleReactionNotifyCommand(EReactionNotifyCommand InNotifyCommand)
{
	if (InNotifyCommand == EReactionNotifyCommand::None || InNotifyCommand == EReactionNotifyCommand::Max)
	{
		FReactionComponentDebug::RecordReactionNotifyCommandIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, InNotifyCommand, TEXT("InvalidCommand"));
		return;
	}

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		FReactionComponentDebug::RecordReactionNotifyCommandIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, InNotifyCommand, TEXT("InvalidExecutor"));
		return;
	}

	if (ActiveReactionContext.IsValidMinimal())
	{
		OnReactionExecutionNotifyCommand.Broadcast(ActiveReactionContext, InNotifyCommand);
	}

	activeExecutor->HandleNotifyCommand(InNotifyCommand);
}

void UCReactionComponent::HandleReactionIncapacitatedPresentationNotify(const EIncapacitatedPresentation InPresentation)
{
	if (InPresentation == EIncapacitatedPresentation::Max)
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, TEXT("IncapacitatedPresentation"), NAME_None, TEXT("InvalidPresentation"));
		return;
	}

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor) || !ActiveReactionContext.IsValidMinimal())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("IncapacitatedPresentation"), NAME_None, TEXT("InvalidExecutorOrContext"));
		return;
	}

	OnReactionIncapacitatedPresentationRequested.Broadcast(ActiveReactionContext, InPresentation);
}

void UCReactionComponent::HandleReactionAllowInterventionWindowBegin(FName InWindowKey)
{
	if (InWindowKey.IsNone())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, TEXT("AllowInterventionWindowBegin"), InWindowKey, TEXT("InvalidWindowKey"));
		return;
	}

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("AllowInterventionWindowBegin"), InWindowKey, TEXT("InvalidExecutor"));
		return;
	}

	activeExecutor->OpenAllowInterventionWindow(InWindowKey);
}

void UCReactionComponent::HandleReactionAllowInterventionWindowEnd(FName InWindowKey)
{
	if (InWindowKey.IsNone())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, TEXT("AllowInterventionWindowEnd"), InWindowKey, TEXT("InvalidWindowKey"));
		return;
	}

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("AllowInterventionWindowEnd"), InWindowKey, TEXT("InvalidExecutor"));
		return;
	}

	activeExecutor->CloseAllowInterventionWindow(InWindowKey);
}

void UCReactionComponent::HandleReactionFeedback(FName InTriggerKey)
{
	if (InTriggerKey.IsNone())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, TEXT("Feedback"), InTriggerKey, TEXT("InvalidTriggerKey"));
		return;
	}

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("Feedback"), InTriggerKey, TEXT("InvalidExecutor"));
		return;
	}

	activeExecutor->HandleNotifyFeedback(EReactionFeedbackTiming::TriggerOnce, InTriggerKey);
}

void UCReactionComponent::HandleReactionFeedbackWindowBegin(FName InTriggerKey)
{
	if (InTriggerKey.IsNone())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, TEXT("FeedbackWindowBegin"), InTriggerKey, TEXT("InvalidTriggerKey"));
		return;
	}

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("FeedbackWindowBegin"), InTriggerKey, TEXT("InvalidExecutor"));
		return;
	}

	activeExecutor->HandleNotifyFeedback(EReactionFeedbackTiming::TriggerWindowBegin, InTriggerKey);
}

void UCReactionComponent::HandleReactionFeedbackWindowEnd(FName InTriggerKey)
{
	if (InTriggerKey.IsNone())
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, ActiveReactionExecutor, TEXT("FeedbackWindowEnd"), InTriggerKey, TEXT("InvalidTriggerKey"));
		return;
	}

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("FeedbackWindowEnd"), InTriggerKey, TEXT("InvalidExecutor"));
		return;
	}

	activeExecutor->HandleNotifyFeedback(EReactionFeedbackTiming::TriggerWindowEnd, InTriggerKey);
}

// Component Reference Validation

bool UCReactionComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ MovementComp_Injected, TEXT("UCMovementComponent") },
		{ StateComp_Injected, TEXT("UCStateComponent") },
		{ HealthComp_Injected, TEXT("UCHealthComponent") },
		{ ObservableOverlayComp_Injected, TEXT("UCObservableOverlayComponent") },
		{ ActionComp_Injected, TEXT("UCActionComponent") },
		{ ReactionFeedbackComp_Injected, TEXT("UCReactionFeedbackComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

bool UCReactionComponent::CancelActiveReactionForSystem()
{
	if (!IsActive()) return true;

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor)) return EndActiveReaction(EReactionFinishReason::Interrupted);

	activeExecutor->Stop(EReactionStopReason::Interrupted);
	return !IsActive();
}

// Runtime Lifecycle

void UCReactionComponent::InitializeReactionRuntime()
{
	BuildReactionRuntimeMaps();
	SetInitialActiveReactionRuntimeState();
}

void UCReactionComponent::UninitializeReactionRuntime()
{
	ResetActiveReactionRuntimeState();
	ClearReactionRuntimeMaps();
}

// Runtime Map

void UCReactionComponent::BuildReactionRuntimeMaps()
{
	BuildReactionDataMap(true);
	BuildReactionExecutorMap(true);
}

void UCReactionComponent::ClearReactionRuntimeMaps()
{
	ReactionExecutorMap.Reset();
	ReactionDataMap.Reset();
}

// Active Runtime State

void UCReactionComponent::SetInitialActiveReactionRuntimeState()
{
	ActiveReactionType = EReactionType::Idle;
}

void UCReactionComponent::ResetActiveReactionRuntimeState()
{
	ActiveReactionType = EReactionType::None;
	ActiveReactionData = FReactionData();
	ActiveReactionExecutor = nullptr;
	ActiveReactionContext = FReactionExecutionContext();
}

// Data Build

void UCReactionComponent::BuildReactionDataMap(bool bRebuildAll)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	// Rebuild clears stale reaction data; append keeps the existing map.

	if (bRebuildAll)
	{
		ReactionDataMap.Reset();
	}

	for (const FReactionData& reactionData : ReactionDatas)
	{
		if (!reactionData.IsValidMinimal())
			continue;

		FReactionDataKey reactionDataKey = reactionData.ReactionDataKey;

		if (ReactionDataMap.Contains(reactionDataKey))
		{
			if (bRebuildAll)
			{
				FReactionComponentDebug::RecordReactionDataDuplicateForAudit(OwnerCharacter_Injected, reactionData, bRebuildAll);
				ReactionDataMap[reactionDataKey] = reactionData;
			}
			else // bRebuildAll == false
			{
				// Duplicate reaction data is skipped unless the map is being rebuilt.
				continue;
			}
		}
		else // Contains(reactionDataKey) == false
		{
			ReactionDataMap.Add(reactionDataKey, reactionData);
		}
	}
}

void UCReactionComponent::BuildReactionExecutorMap(bool bRebuildAll)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	// Rebuild clears stale reaction executors; append keeps existing cache entries.

	if (bRebuildAll)
	{
		ReactionExecutorMap.Reset();
	}

	for (const FReactionData& reactionData : ReactionDatas)
	{
		if (!reactionData.IsValidMinimal()) continue;

		UClass* executorKey = reactionData.ReactionExecutorKey.Get();
		if (!IsValid(executorKey)) continue;

		// Preferred: keep existing cached reaction executor.
		if (!bRebuildAll)
		{
			const UCReaction* found = FindReactionExecutor(executorKey);
			if (IsValid(found)) continue;
		}

		// Fallback: create cached reaction executor.
		UCReaction* add = AddReactionExecutor(executorKey);
		if (!IsValid(add))
		{
			FReactionComponentDebug::RecordReactionExecutorMapBuildFailedForAudit(OwnerCharacter_Injected, reactionData, TEXT("AddFailed"));
			continue;
		}
	}
}

FCharacterComponentReferences UCReactionComponent::BuildReactionExecutorReferences()
{
	FCharacterComponentReferences references;

	references.OwnerCharacter = OwnerCharacter_Injected;
	references.ReactionComponent = this;
	references.ReactionFeedbackComponent = ReactionFeedbackComp_Injected;

	return references;
}

UCReaction* UCReactionComponent::AddReactionExecutor(const TSubclassOf<class UCReaction> InSubClass)
{
	UClass* executorKey = InSubClass.Get();
	if (!IsValid(executorKey)) return nullptr;

	UCReaction* add = NewObject<UCReaction>(this, InSubClass);
	if (!IsValid(add)) return nullptr;

	const FCharacterComponentReferences references = BuildReactionExecutorReferences();
	add->InitializeReferences(references);
	ReactionExecutorMap.Add(executorKey, add);

	return add;
}

UCReaction* UCReactionComponent::FindReactionExecutor(const UClass* InClass)
{
	UCReaction** foundPtr = ReactionExecutorMap.Find(InClass);
	if (!foundPtr) return nullptr;

	UCReaction* found = *foundPtr;

	if (!IsValid(found))
	{
		ReactionExecutorMap.Remove(InClass);

		return nullptr;
	}

	return found;
}

// Data Resolve - Match Mode

bool UCReactionComponent::ResolveGlobalReactionData(const FReactionDataKey& InDataKey, FReactionData& OutData)
{
	const FReactionData* foundPtr = ReactionDataMap.Find(InDataKey);
	if (!foundPtr || !foundPtr->IsValidMinimal())
	{
		FReactionComponentDebug::RecordReactionDataResolveFailedForAudit(OwnerCharacter_Injected, InDataKey, TEXT("GlobalNotFound"));
		return false;
	}

	OutData = *foundPtr;
	FReactionComponentDebug::RecordReactionDataResolvedForAudit(OwnerCharacter_Injected, InDataKey, OutData, 0);
	return true;
}

bool UCReactionComponent::ResolveDamageSpecReactionData(const FReactionDataKey& InDataKey, FReactionData& OutData)
{
	TArray<FDamageSpecKey> candidateKeys;
	const EReactionType reactionType = InDataKey.ReactionType;

	BuildCandidateSpecKeys(InDataKey.DamageSpecKey, candidateKeys);

	for (int32 candidateIndex = 0; candidateIndex < candidateKeys.Num(); ++candidateIndex)
	{
		const FDamageSpecKey& candidateKey = candidateKeys[candidateIndex];
		FReactionDataKey reactionDataKey;

		reactionDataKey.MatchMode = EReactionDataMatchMode::DamageSpec;
		reactionDataKey.DamageSpecKey = candidateKey;
		reactionDataKey.ReactionType = reactionType;
		reactionDataKey.ReactionIndex = InDataKey.ReactionIndex;

		const FReactionData* foundPtr = ReactionDataMap.Find(reactionDataKey);
		if (!foundPtr) continue;

		const FReactionData& found = *foundPtr;
		if (!found.IsValidMinimal())
		{
			FReactionComponentDebug::RecordReactionDataResolveFailedForAudit(OwnerCharacter_Injected, reactionDataKey, TEXT("InvalidResolvedData"));
			continue;
		}

		OutData = found;
		FReactionComponentDebug::RecordReactionDataResolvedForAudit(OwnerCharacter_Injected, InDataKey, found, candidateIndex);
		return true;
	}

	FReactionComponentDebug::RecordReactionDataResolveFailedForAudit(OwnerCharacter_Injected, InDataKey, TEXT("NotFound"));
	return false;
}

// Data Resolve - DamageSpec Fallback

void UCReactionComponent::BuildCandidateSpecKeys(const FDamageSpecKey& InSpecKey, TArray<FDamageSpecKey>& OutSpecKeys) const
{
	OutSpecKeys.Reset();

	// Exact: weapon, action, and index.
	OutSpecKeys.Add(InSpecKey);

	// Fallback: weapon and action with any index.
	{
		FDamageSpecKey candidateKey = InSpecKey;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}

	// Fallback: weapon with any action and any index.
	{
		FDamageSpecKey candidateKey = InSpecKey;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}

	// Final fallback: any weapon, any action, and any index.
	{
		FDamageSpecKey candidateKey = InSpecKey;
		candidateKey.WeaponType = EWeaponType::All;
		candidateKey.ActionType = EActionType::All;
		candidateKey.ActionIndex = INDEX_NONE;
		OutSpecKeys.Add(candidateKey);
	}
}

// Decision Apply

bool UCReactionComponent::ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsRequested()) return true;
	if (!InDirective.IsValidRequest())
	{
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, FReactionExecutionContext(), TEXT("ApplyIntervention"), TEXT("InvalidDirective"));
		return false;
	}

	switch (InDirective.TargetDomain)
	{
	case EExecutionDomain::Action:
	{
		bool bApplied = IsValid(ActionComp_Injected) && ActionComp_Injected->RequestInterruptActiveAction(InDirective);
		if (!bApplied)
		{
			FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, FReactionExecutionContext(), TEXT("ApplyIntervention"), TEXT("ActionInterventionFailed"));
		}
		return bApplied;
	}

	case EExecutionDomain::Reaction:
		return InterruptActiveReaction(InDirective);

	default:
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, FReactionExecutionContext(), TEXT("ApplyIntervention"), TEXT("UnsupportedTargetDomain"));
		return false;
	}
}

bool UCReactionComponent::ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	if (InHandlings.IsEmpty()) return true;

	return IsValid(ObservableOverlayComp_Injected) && ObservableOverlayComp_Injected->ApplyOverlayHandlings(InHandlings);
}

// Execution Operations

bool UCReactionComponent::StartReaction(const FReactionExecutionContext& InContext)
{
	if (IsActive())
	{
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("Start"), TEXT("AlreadyActive"));
		return false;
	}
	if (!InContext.IsValidMinimal())
	{
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("Start"), TEXT("InvalidContext"));
		return false;
	}

	UCReaction* incomingExecutor = InContext.ReactionExecutor;
	if (!IsValid(incomingExecutor))
	{
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("Start"), TEXT("InvalidExecutor"));
		return false;
	}

	const FReactionData& incomingData = InContext.ReactionData;

	EnterReactionState(incomingData);

	if (!incomingExecutor->Start(incomingData))
	{
		ExitReactionState(incomingData);
		FReactionComponentDebug::RecordReactionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("Start"), TEXT("ExecutorStartFailed"));
		return false;
	}

	SetActiveReactionContext(InContext);
	BroadcastReactionExecutionLifecycleEvent(EReactionExecutionLifecycleEventType::Started, EReactionFinishReason::None, InContext);
	FReactionComponentDebug::RecordReactionRuntimeAcceptedForAudit(OwnerCharacter_Injected, InContext, TEXT("Start"));
	return true;
}

bool UCReactionComponent::InterruptActiveReaction(const FExecutionInterventionDirective& InDirective)
{
	if (!IsActive()) return true;

	const EReactionFinishReason finishReason = ConvertExecutionStopReasonToReactionFinishReason(InDirective.StopReason);

	UCReaction* activeExecutor = GetActiveReactionExecutor();
	if (!IsValid(activeExecutor))
	{
		// Force end when the active executor is already invalid.
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("Interrupt"), NAME_None, TEXT("InvalidExecutorFallbackEnd"));
		return EndActiveReaction(finishReason);
	}

	activeExecutor->Interrupt(InDirective);

	if (IsActive())
	{
		// Force end when the executor interrupt callback did not clear active state.
		FReactionComponentDebug::RecordReactionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("Interrupt"), NAME_None, TEXT("ExecutorDidNotEndFallbackEnd"));
		return EndActiveReaction(finishReason);
	}

	return !IsActive();
}

bool UCReactionComponent::EndActiveReaction(EReactionFinishReason InFinishReason)
{
	if (!IsActive()) return true;

	const FReactionData activeData = ActiveReactionData;
	const FReactionExecutionContext activeContext = ActiveReactionContext;

	if (activeData.IsValidMinimal())
	{
		ExitReactionState(activeData);
	}

	ClearActiveReactionContext();

	EReactionExecutionLifecycleEventType lifecycleEventType = EReactionExecutionLifecycleEventType::Ignored;
	if (InFinishReason == EReactionFinishReason::Completed)
	{
		lifecycleEventType = EReactionExecutionLifecycleEventType::Completed;
	}
	else if (InFinishReason == EReactionFinishReason::Interrupted)
	{
		lifecycleEventType = EReactionExecutionLifecycleEventType::Interrupted;
	}
	BroadcastReactionExecutionLifecycleEvent(lifecycleEventType, InFinishReason, activeContext);

	FReactionComponentDebug::RecordReactionRuntimeAcceptedForAudit(OwnerCharacter_Injected, activeContext, TEXT("End"));

	return !IsActive();
}

// Active Context

void UCReactionComponent::SetActiveReactionContext(const FReactionExecutionContext& InContext)
{
	if (!InContext.IsValidMinimal()) return;

	const EReactionType prevReactionType = ActiveReactionType;

	ActiveReactionType = InContext.ReactionDataKey.ReactionType;
	ActiveReactionData = InContext.ReactionData;
	ActiveReactionExecutor = InContext.ReactionExecutor;
	ActiveReactionContext = InContext;

	if (OnReactionTypeChanged.IsBound())
	{
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Injected, prevReactionType, ActiveReactionType);
	}
}

void UCReactionComponent::ClearActiveReactionContext()
{
	const EReactionType prevReactionType = ActiveReactionType;

	ActiveReactionType = EReactionType::None;
	ActiveReactionData = FReactionData();
	ActiveReactionExecutor = nullptr;
	ActiveReactionContext = FReactionExecutionContext();

	if (OnReactionTypeChanged.IsBound())
	{
		OnReactionTypeChanged.Broadcast(OwnerCharacter_Injected, prevReactionType, ActiveReactionType);
	}
}

void UCReactionComponent::BroadcastReactionExecutionLifecycleEvent(EReactionExecutionLifecycleEventType InEventType, EReactionFinishReason InFinishReason, const FReactionExecutionContext& InContext)
{
	if (!OnReactionExecutionLifecycleEvent.IsBound()) return;

	FReactionExecutionLifecycleEvent event;
	event.EventType = InEventType;
	event.FinishReason = InFinishReason;
	event.Context = InContext;
	OnReactionExecutionLifecycleEvent.Broadcast(event);
}

// State Transition

void UCReactionComponent::EnterReactionState(const FReactionData& InData)
{
	if (IsValid(MovementComp_Injected) && !InData.bCanMove)
	{
		MovementComp_Injected->SetMovementEnabled(false);
	}

	if (IsValid(StateComp_Injected))
	{
		StateComp_Injected->SetReactionState();
	}
}

void UCReactionComponent::ExitReactionState(const FReactionData& InData)
{
	if (IsValid(MovementComp_Injected) && !InData.bCanMove)
	{
		MovementComp_Injected->SetMovementEnabled(true);
	}

	if (IsValid(StateComp_Injected))
	{
		StateComp_Injected->SetIdleState();
	}
}

// Conversion

EReactionFinishReason UCReactionComponent::ConvertExecutionStopReasonToReactionFinishReason(EExecutionStopReason InStopReason) const
{
	switch (InStopReason)
	{
	case EExecutionStopReason::Interrupted:
		return EReactionFinishReason::Interrupted;

	default:
		return EReactionFinishReason::Ignored;
	}
}
