#include "Component/CActionComponent.h"

#include "ProjectGlobal.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Action/CAction.h"
#include "Type/CActionTypes.h"
#include "Type/CActionDataTypes.h"
#include "Type/CActionFeedbackTypes.h"
#include "Type/CActionOrchestrationTypes.h"
#include "Type/CObservableOverlayTypes.h"
#include "Type/CExecutionTypes.h"
#include "Core/Debug/FActionComponentDebug.h"
#include "Core/Profiling/CCombatCollisionProfilingCounters.h"

#include "GameFramework/Character.h"

UCActionComponent::UCActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCActionComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	MovementComp_Injected = InReferences.MovementComponent;
	WeaponComp_Injected = InReferences.WeaponComponent;
	StateComp_Injected = InReferences.StateComponent;
	HealthComp_Injected = InReferences.HealthComponent;
	ObservableOverlayComp_Injected = InReferences.ObservableOverlayComponent;
	CombatSignalSourceComp_Injected = InReferences.CombatSignalSourceComponent;
	ActionOrchestratorComp_Injected = InReferences.ActionOrchestratorComponent;
	ReactionComp_Injected = InReferences.ReactionComponent;
	ActionFeedbackComp_Injected = InReferences.ActionFeedbackComponent;

	ValidateRequiredComponentReferences();
}

bool UCActionComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ MovementComp_Injected, TEXT("UCMovementComponent") },
		{ WeaponComp_Injected, TEXT("UCWeaponComponent") },
		{ StateComp_Injected, TEXT("UCStateComponent") },
		{ HealthComp_Injected, TEXT("UCHealthComponent") },
		{ ObservableOverlayComp_Injected, TEXT("UCObservableOverlayComponent") },
		{ CombatSignalSourceComp_Injected, TEXT("UCCombatSignalSourceComponent") },
		{ ActionOrchestratorComp_Injected, TEXT("UCActionOrchestratorComponent") },
		{ ReactionComp_Injected, TEXT("UCReactionComponent") },
		{ ActionFeedbackComp_Injected, TEXT("UCActionFeedbackComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Lifecycle

void UCActionComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeActionRuntime();
}

void UCActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeActionRuntime();

	Super::EndPlay(EndPlayReason);
}

void UCActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TPair<UClass*, UCAction*>& pair : ActionExecutorMap)
	{
		UCAction* actionExecutor = pair.Value;
		if (!IsValid(actionExecutor)) continue;

		actionExecutor->Tick(DeltaTime);
	}
}

// Runtime Lifecycle

void UCActionComponent::InitializeActionRuntime()
{
	BuildActionRuntimeMaps();
	SetInitialActiveActionRuntimeState();
}

void UCActionComponent::UninitializeActionRuntime()
{
	ResetActiveActionRuntimeState();
	ClearActionRuntimeMaps();
}

// Runtime Map

void UCActionComponent::BuildActionRuntimeMaps()
{
	BuildActionDataMap(true);
	BuildActionExecutorMap(true);
}

void UCActionComponent::ClearActionRuntimeMaps()
{
	ActionExecutorMap.Reset();
	ActionDataMap.Reset();
}

// Active Runtime State

void UCActionComponent::SetInitialActiveActionRuntimeState()
{
	ActiveActionType = EActionType::Idle;
}

void UCActionComponent::ResetActiveActionRuntimeState()
{
	ActiveActionType = EActionType::None;
	ActiveActionIndex = INDEX_NONE;
	ActiveActionData = FActionData();
	ActiveActionExecutor = nullptr;
}

// Query

bool UCActionComponent::IsActive() const
{
	return ActiveActionType != EActionType::None
		&& ActiveActionType != EActionType::Idle
		&& ActiveActionType != EActionType::All
		&& ActiveActionType != EActionType::Max;
}

EActionType UCActionComponent::GetActiveActionType() const
{
	return ActiveActionType;
}

int32 UCActionComponent::GetActiveActionIndex() const
{
	return ActiveActionIndex;
}

bool UCActionComponent::GetActiveActionData(FActionData& OutData) const
{
	OutData = FActionData();

	if (!IsActive()) return false;
	if (!ActiveActionData.IsValidMinimal()) return false;

	OutData = ActiveActionData;
	return true;
}

UCAction* UCActionComponent::GetActiveActionExecutor() const
{
	if (!IsActive()) return nullptr;
	if (!IsValid(ActiveActionExecutor)) return nullptr;

	return ActiveActionExecutor;
}

// Data Resolve

bool UCActionComponent::ResolveActionData(const FActionDataKey& InDataKey, FActionData& OutData)
{
	OutData = FActionData();

	if (!InDataKey.IsValidMinimal())
	{
		FActionComponentDebug::RecordActionDataResolveFailedForAudit(OwnerCharacter_Injected, InDataKey, TEXT("InvalidDataKey"));
		return false;
	}

	FActionData const* foundPtr = ActionDataMap.Find(InDataKey);
	if (!foundPtr)
	{
		FActionComponentDebug::RecordActionDataResolveFailedForAudit(OwnerCharacter_Injected, InDataKey, TEXT("DataNotFound"));
		return false;
	}

	FActionData found = *foundPtr;
	if (!found.IsValidMinimal())
	{
		FActionComponentDebug::RecordActionDataResolveFailedForAudit(OwnerCharacter_Injected, InDataKey, TEXT("InvalidData"));
		return false;
	}

	OutData = found;
	return true;
}

UCAction* UCActionComponent::ResolveActionExecutor(const FActionData& InData)
{
	// Preferred: reuse cached action executor.
	UCAction* found = FindActionExecutor(InData.ActionExecutorKey.Get());
	if (IsValid(found)) return found;

	// Fallback: create and cache action executor.
	UCAction* add = AddActionExecutor(InData.ActionExecutorKey);
	if (IsValid(add)) return add;

	FActionComponentDebug::RecordActionExecutorResolveFailedForAudit(OwnerCharacter_Injected, InData, TEXT("AddExecutorFailed"));
	return nullptr;
}

bool UCActionComponent::CanCommitChain(const UCAction* InAction, const FActionData& InData) const
{
	if (!IsActive()) return false;

	if (!IsValid(InAction)) return false;
	if (InAction != GetActiveActionExecutor()) return false;
	if (!InData.IsValidMinimal()) return false;

	if (!IsValid(HealthComp_Injected) || !HealthComp_Injected->IsAlive()) return false;

	if (!IsValid(StateComp_Injected)) return false;
	if (StateComp_Injected->GetCurrentExecutionState() != EExecutionState::Action) return false;

	return true;
}

// Execution Entry

bool UCActionComponent::ApplyActionDecision(const FActionExecutionResult& InResult)
{
	if (!IsValid(OwnerCharacter_Injected))
	{
		FActionComponentDebug::RecordActionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("InvalidOwner"));
		return false;
	}

	if (!InResult.IsAcceptedDecision())
	{
		FActionComponentDebug::RecordActionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("RejectedDecision"));
		return false;
	}

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
	{
		if (!ApplyOverlayHandlings(InResult.OverlayHandlings))
		{
			FActionComponentDebug::RecordActionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("OverlayHandlingFailed"));
			return false;
		}

		bool bStarted = StartAction(InResult.ResolvedContext);
		if (bStarted)
		{
			FActionComponentDebug::RecordActionDecisionAppliedForAudit(OwnerCharacter_Injected, InResult, TEXT("Start"));
		}
		return bStarted;
	}

	case EExecutionApplyMode::Reserve:
	{
		bool bReserved = ReserveAction(InResult.ResolvedContext);
		if (bReserved)
		{
			FActionComponentDebug::RecordActionDecisionAppliedForAudit(OwnerCharacter_Injected, InResult, TEXT("Reserve"));
		}
		return bReserved;
	}

	case EExecutionApplyMode::Intervene:
	{
		if (!ApplyExecutionInterventionDirective(InResult.InterventionDirective))
		{
			FActionComponentDebug::RecordActionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("InterventionFailed"));
			return false;
		}

		if (!ApplyOverlayHandlings(InResult.OverlayHandlings))
		{
			FActionComponentDebug::RecordActionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("OverlayHandlingFailed"));
			return false;
		}

		bool bStarted = StartAction(InResult.ResolvedContext);
		if (bStarted)
		{
			FActionComponentDebug::RecordActionDecisionAppliedForAudit(OwnerCharacter_Injected, InResult, TEXT("Intervene"));
		}
		return bStarted;
	}

	default:
		FActionComponentDebug::RecordActionDecisionRejectedForAudit(OwnerCharacter_Injected, InResult, TEXT("ApplyDecision"), TEXT("InvalidApplyMode"));
		return false;
	}
}

bool UCActionComponent::RequestInterruptActiveAction(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsValidRequest()) return false;
	if (InDirective.TargetDomain != EExecutionDomain::Action) return false;

	return InterruptActiveAction(InDirective);
}

// Execution Result Hooks

bool UCActionComponent::HandleApplyActionConsumed(const UCAction* InAction, const FActionData& InData)
{
	if (!IsActive()) return false;
	if (!IsValid(InAction)) return false;
	if (InAction != GetActiveActionExecutor()) return false;
	if (!InData.IsValidMinimal()) return false;

	ActiveActionType = InData.ActionDataKey.ActionType;
	ActiveActionIndex = InData.ActionDataKey.ActionIndex;
	ActiveActionData = InData;

	return true;
}

void UCActionComponent::HandleApplyActionFinished(const UCAction* InAction, EActionFinishReason InFinishReason)
{
	if (!IsActive()) return;
	if (!IsValid(InAction)) return;
	if (InAction != GetActiveActionExecutor()) return;

	EndActiveAction(InFinishReason);
}

// Notify Routing

void UCActionComponent::HandleActionNotifyCommand(EActionNotifyCommand InNotifyCommand)
{
	if (InNotifyCommand == EActionNotifyCommand::None || InNotifyCommand == EActionNotifyCommand::Max)
	{
		FActionComponentDebug::RecordActionNotifyCommandIgnoredForAudit(OwnerCharacter_Injected, nullptr, InNotifyCommand, TEXT("InvalidCommand"));
		return;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionNotifyCommandIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, InNotifyCommand, TEXT("NoActiveExecutor"));
		return;
	}

	activeExecutor->HandleNotifyCommand(InNotifyCommand);
}

void UCActionComponent::HandleActionAllowInterventionWindowBegin(FName InWindowKey)
{
	if (InWindowKey.IsNone())
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, nullptr, TEXT("AllowInterventionWindowBegin"), InWindowKey, TEXT("InvalidWindowKey"));
		return;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("AllowInterventionWindowBegin"), InWindowKey, TEXT("NoActiveExecutor"));
		return;
	}

	activeExecutor->OpenAllowInterventionWindow(InWindowKey);
}

void UCActionComponent::HandleActionAllowInterventionWindowEnd(FName InWindowKey)
{
	if (InWindowKey.IsNone())
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, nullptr, TEXT("AllowInterventionWindowEnd"), InWindowKey, TEXT("InvalidWindowKey"));
		return;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("AllowInterventionWindowEnd"), InWindowKey, TEXT("NoActiveExecutor"));
		return;
	}

	activeExecutor->CloseAllowInterventionWindow(InWindowKey);
}

void UCActionComponent::HandleActionFeedback(FName InTriggerKey)
{
	if (InTriggerKey.IsNone())
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, nullptr, TEXT("Feedback"), InTriggerKey, TEXT("InvalidTriggerKey"));
		return;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("Feedback"), InTriggerKey, TEXT("NoActiveExecutor"));
		return;
	}

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerOnce, InTriggerKey);
}

void UCActionComponent::HandleActionFeedbackWindowBegin(FName InTriggerKey)
{
	if (InTriggerKey.IsNone())
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, nullptr, TEXT("FeedbackWindowBegin"), InTriggerKey, TEXT("InvalidTriggerKey"));
		return;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("FeedbackWindowBegin"), InTriggerKey, TEXT("NoActiveExecutor"));
		return;
	}

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerWindowBegin, InTriggerKey);
}

void UCActionComponent::HandleActionFeedbackWindowEnd(FName InTriggerKey)
{
	if (InTriggerKey.IsNone())
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, nullptr, TEXT("FeedbackWindowEnd"), InTriggerKey, TEXT("InvalidTriggerKey"));
		return;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("FeedbackWindowEnd"), InTriggerKey, TEXT("NoActiveExecutor"));
		return;
	}

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerWindowEnd, InTriggerKey);
}

void UCActionComponent::HandleActionCollisionWindowBegin(FName InCollisionName)
{
	if (!IsValid(WeaponComp_Injected))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, nullptr, TEXT("CollisionWindowBegin"), InCollisionName, TEXT("InvalidWeaponComponent"));
		return;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, activeExecutor, TEXT("CollisionWindowBegin"), InCollisionName, TEXT("NoActiveExecutor"));
		return;
	}

	FCombatCollisionProfilingCounters::RecordActionCollisionWindowBegin();

	WeaponComp_Injected->OpenCollisionWindow(InCollisionName);
}

void UCActionComponent::HandleActionCollisionWindowEnd()
{
	if (!IsValid(WeaponComp_Injected))
	{
		FActionComponentDebug::RecordActionNotifyIgnoredForAudit(OwnerCharacter_Injected, nullptr, TEXT("CollisionWindowEnd"), NAME_None, TEXT("InvalidWeaponComponent"));
		return;
	}

	FCombatCollisionProfilingCounters::RecordActionCollisionWindowEnd();

	WeaponComp_Injected->CloseCollisionWindow();
}

bool UCActionComponent::HandleActionCombatSignalCue(FName InCueTag)
{
	if (InCueTag.IsNone())
	{
		FActionComponentDebug::RecordActionCombatSignalCueForAudit(OwnerCharacter_Injected, nullptr, InCueTag, TEXT("Ignored"), TEXT("InvalidCueTag"));
		return false;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionCombatSignalCueForAudit(OwnerCharacter_Injected, activeExecutor, InCueTag, TEXT("Ignored"), TEXT("NoActiveExecutor"));
		return false;
	}

	FCombatCollisionProfilingCounters::RecordActionCombatSignalCue();

	FActionCombatSignalCueResolution resolution;
	if (!activeExecutor->ResolveNotifyCombatSignalCue(InCueTag, resolution))
	{
		FActionComponentDebug::RecordActionCombatSignalCueForAudit(OwnerCharacter_Injected, activeExecutor, InCueTag, TEXT("Rejected"), TEXT("ResolveCueFailed"));
		return false;
	}

	if (!resolution.IsValidResolution())
	{
		FActionComponentDebug::RecordActionCombatSignalCueForAudit(OwnerCharacter_Injected, activeExecutor, InCueTag, TEXT("Rejected"), TEXT("InvalidCueResolution"));
		return false;
	}

	if (!IsValid(CombatSignalSourceComp_Injected))
	{
		FActionComponentDebug::RecordActionCombatSignalCueForAudit(OwnerCharacter_Injected, activeExecutor, InCueTag, TEXT("Rejected"), TEXT("MissingCombatSignalSourceComponent"));
		return false;
	}

	bool bRequested = CombatSignalSourceComp_Injected->RequestAICombatSignalCue(resolution.CueTag);
	FActionComponentDebug::RecordActionCombatSignalCueForAudit(OwnerCharacter_Injected, activeExecutor, resolution.CueTag, bRequested ? TEXT("Accepted") : TEXT("Rejected"), bRequested ? nullptr : TEXT("SourceRejectedCue"));
	return bRequested;
}

// Cross-System Dispatch

bool UCActionComponent::ApplyOverlayEvent(const FObservableOverlayEventContext& InContext)
{
	return IsValid(ObservableOverlayComp_Injected) && ObservableOverlayComp_Injected->ApplyOverlayEvent(InContext);
}

FActionRequestResult UCActionComponent::ConsumeDeferredAction(EDeferredActionConsumeKey InConsumeKey)
{
	if (!IsValid(ActionOrchestratorComp_Injected)) return FActionRequestResult();

	return ActionOrchestratorComp_Injected->ConsumeDeferredAction(InConsumeKey);
}

void UCActionComponent::ClearDeferredActions(EDeferredActionConsumeKey InConsumeKey)
{
	if (IsValid(ActionOrchestratorComp_Injected))
	{
		ActionOrchestratorComp_Injected->ClearDeferredActions(InConsumeKey);
	}
}

// Event Broadcast

void UCActionComponent::BroadcastActionEvent(EActionType InType, int32 InIndex, EActionEventType InEventType)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	if (OnActionEvent.IsBound())
	{
		OnActionEvent.Broadcast(OwnerCharacter_Injected, InType, InIndex, InEventType);
	}
}

// Data Build

void UCActionComponent::BuildActionDataMap(bool bRebuildAll)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	// Rebuild clears stale action data; append keeps the existing map.

	if (bRebuildAll)
	{
		ActionDataMap.Reset();
	}

	for (const FActionData& actionData : ActionDatas)
	{
		if (!actionData.IsValidMinimal()) continue;

		const FActionDataKey& actionDataKey = actionData.ActionDataKey;
		if (!actionDataKey.IsValidMinimal()) continue;

		if (ActionDataMap.Contains(actionDataKey))
		{
			if (bRebuildAll)
			{
				FActionComponentDebug::RecordActionDataDuplicateForAudit(OwnerCharacter_Injected, actionData, bRebuildAll);
				ActionDataMap[actionDataKey] = actionData;
			}
			else // bRebuildAll == false
			{
				// Duplicate action data is skipped unless the map is being rebuilt.
				continue;
			}
		}
		else // Contains(actionDataKey) == false
		{
			ActionDataMap.Add(actionDataKey, actionData);
		}
	}
}

void UCActionComponent::BuildActionExecutorMap(bool bRebuildAll)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	// Rebuild clears stale action executors; append keeps existing cache entries.

	if (bRebuildAll)
	{
		ActionExecutorMap.Reset();
	}

	for (const FActionData& actionData : ActionDatas)
	{
		if (!actionData.IsValidMinimal()) continue;

		UClass* executorKey = actionData.ActionExecutorKey.Get();
		if (!IsValid(executorKey)) continue;

		// Preferred: keep existing cached action executor.
		if (!bRebuildAll)
		{
			const UCAction* found = FindActionExecutor(executorKey);
			if (IsValid(found)) continue;
		}

		// Fallback: create cached action executor.
		UCAction* add = AddActionExecutor(executorKey);
		if (!IsValid(add))
		{
			FActionComponentDebug::RecordActionExecutorMapBuildFailedForAudit(OwnerCharacter_Injected, actionData, TEXT("AddExecutorFailed"));
			continue;
		}
	}
}

FCharacterComponentReferences UCActionComponent::BuildActionExecutorReferences()
{
	FCharacterComponentReferences references;

	references.OwnerCharacter = OwnerCharacter_Injected;
	references.WeaponComponent = WeaponComp_Injected;
	references.ActionComponent = this;
	references.ActionFeedbackComponent = ActionFeedbackComp_Injected;

	return references;
}

UCAction* UCActionComponent::AddActionExecutor(const TSubclassOf<class UCAction> InSubClass)
{
	UClass* executorKey = InSubClass.Get();
	if (!IsValid(executorKey)) return nullptr;

	UCAction* add = NewObject<UCAction>(this, InSubClass);
	if (!IsValid(add)) return nullptr;

	const FCharacterComponentReferences references = BuildActionExecutorReferences();
	add->InitializeReferences(references);
	ActionExecutorMap.Add(executorKey, add);

	return add;
}

UCAction* UCActionComponent::FindActionExecutor(const UClass* InClass)
{
	UCAction* const* foundPtr = ActionExecutorMap.Find(InClass);
	if (!foundPtr) return nullptr;

	UCAction* found = *foundPtr;

	if (!IsValid(found))
	{
		ActionExecutorMap.Remove(InClass);

		return nullptr;
	}

	return found;
}

// Decision Apply

bool UCActionComponent::ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsRequested()) return true;
	if (!InDirective.IsValidRequest()) return false;

	switch (InDirective.TargetDomain)
	{
	case EExecutionDomain::Action:
		return InterruptActiveAction(InDirective);

	case EExecutionDomain::Reaction:
		return IsValid(ReactionComp_Injected) && ReactionComp_Injected->RequestInterruptActiveReaction(InDirective);

	default:
		return false;
	}
}

bool UCActionComponent::ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	if (InHandlings.IsEmpty()) return true;
	return IsValid(ObservableOverlayComp_Injected) && ObservableOverlayComp_Injected->ApplyOverlayHandlings(InHandlings);
}

// Execution Operations

bool UCActionComponent::StartAction(const FActionExecutionContext& InContext)
{
	if (IsActive())
	{
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("StartAction"), TEXT("AlreadyActive"));
		return false;
	}

	if (!InContext.IsValidMinimal())
	{
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("StartAction"), TEXT("InvalidContext"));
		return false;
	}

	UCAction* incomingExecutor = InContext.ActionExecutor;
	if (!IsValid(incomingExecutor))
	{
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("StartAction"), TEXT("InvalidExecutor"));
		return false;
	}

	const FActionData& incomingData = InContext.ActionData;

	EnterActionState(incomingData);

	if (!incomingExecutor->Start(incomingData))
	{
		ExitActionState(incomingData);
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("StartAction"), TEXT("ExecutorStartFailed"));
		return false;
	}

	SetActiveActionContext(InContext);
	FActionComponentDebug::PrintActionExecutionContextDebug(OwnerCharacter_Injected, InContext, TEXT("StartAction"));
	return true;
}

bool UCActionComponent::ReserveAction(const FActionExecutionContext& InContext)
{
	if (!IsActive())
	{
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("ReserveAction"), TEXT("NoActiveAction"));
		return false;
	}

	if (!InContext.IsValidMinimal())
	{
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("ReserveAction"), TEXT("InvalidContext"));
		return false;
	}

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("ReserveAction"), TEXT("NoActiveExecutor"));
		return false;
	}

	const FActionData& incomingData = InContext.ActionData;

	bool bReserved = activeExecutor->ReserveChain(incomingData);
	if (!bReserved)
	{
		FActionComponentDebug::RecordActionRuntimeRejectedForAudit(OwnerCharacter_Injected, InContext, TEXT("ReserveAction"), TEXT("ExecutorReserveFailed"));
	}
	return bReserved;
}

bool UCActionComponent::InterruptActiveAction(const FExecutionInterventionDirective& InDirective)
{
	if (!IsActive()) return true;

	const EActionFinishReason finishReason = ConvertExecutionStopReasonToActionFinishReason(InDirective.StopReason);

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		// Force end when the active executor is already invalid.
		return EndActiveAction(finishReason);
	}

	activeExecutor->Interrupt(InDirective);

	if (IsActive())
	{
		// Force end when the executor interrupt callback did not clear active state.
		return EndActiveAction(finishReason);
	}

	return true;
}

bool UCActionComponent::EndActiveAction(EActionFinishReason InFinishReason)
{
	if (!IsActive()) return true;

	const FActionData activeData = ActiveActionData;

	if (activeData.IsValidMinimal())
	{
		ExitActionState(activeData);
	}

	ClearActiveActionContext();

	return !IsActive();
}

// Active Context

void UCActionComponent::SetActiveActionContext(const FActionExecutionContext& InContext)
{
	if (!InContext.IsValidMinimal()) return;

	const EActionType prevActionType = ActiveActionType;

	ActiveActionType = InContext.ActionData.ActionDataKey.ActionType;
	ActiveActionIndex = InContext.ActionData.ActionDataKey.ActionIndex;
	ActiveActionData = InContext.ActionData;
	ActiveActionExecutor = InContext.ActionExecutor;

	if (OnActionTypeChanged.IsBound())
	{
		OnActionTypeChanged.Broadcast(OwnerCharacter_Injected, prevActionType, ActiveActionType);
	}
}

void UCActionComponent::ClearActiveActionContext()
{
	const EActionType prevActionType = ActiveActionType;

	ActiveActionType = EActionType::None;
	ActiveActionIndex = INDEX_NONE;
	ActiveActionData = FActionData();
	ActiveActionExecutor = nullptr;

	if (OnActionTypeChanged.IsBound())
	{
		OnActionTypeChanged.Broadcast(OwnerCharacter_Injected, prevActionType, ActiveActionType);
	}
}

// State Transition

void UCActionComponent::EnterActionState(const FActionData& InData)
{
	if (IsValid(MovementComp_Injected) && !InData.bCanMove)
	{
		MovementComp_Injected->SetStop();
	}

	if (IsValid(StateComp_Injected))
	{
		StateComp_Injected->SetActionState();
	}
}

void UCActionComponent::ExitActionState(const FActionData& InData)
{
	if (IsValid(MovementComp_Injected) && !InData.bCanMove)
	{
		MovementComp_Injected->SetMove();
	}

	if (IsValid(StateComp_Injected))
	{
		StateComp_Injected->SetIdleState();
	}
}

// Conversion

EActionFinishReason UCActionComponent::ConvertExecutionStopReasonToActionFinishReason(EExecutionStopReason InStopReason) const
{
	switch (InStopReason)
	{
	case EExecutionStopReason::Interrupted:
		return EActionFinishReason::Interrupted;

	case EExecutionStopReason::Ignored:
	default:
		return EActionFinishReason::Ignored;
	}
}
