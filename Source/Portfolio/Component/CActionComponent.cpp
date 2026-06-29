#include "Component/CActionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

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

#include "Type/CWeaponStructure.h"

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

	// Rebuild All
	BuildActionDataMap(true);
	BuildActionExecutorMap(true);

	// Init Action State
	ActiveActionType = EActionType::Idle;
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

	if (!InDataKey.IsValidMinimal()) return false;

	FActionData const* foundPtr = ActionDataMap.Find(InDataKey);
	if (!foundPtr) return false;

	FActionData found = *foundPtr;
	if (!found.IsValidMinimal()) return false;

	OutData = found;
	return true;
}

UCAction* UCActionComponent::ResolveActionExecutor(const FActionData& InData)
{
	// 1) Try reuse cached Action; return if valid
	UCAction* found = FindActionExecutor(InData.ActionExecutorKey.Get());
	if (IsValid(found)) return found;

	// 2) [Policy] Try Add and cache Action; return if valid
	UCAction* add = AddActionExecutor(InData.ActionExecutorKey);
	if (IsValid(add)) return add;

	// [Debug] ActionData is Valid; but Find and Add Failed
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
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!InResult.IsAcceptedDecision()) return false;

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
	{
		if (!ApplyOverlayHandlings(InResult.OverlayHandlings)) return false;

		return StartAction(InResult.ResolvedContext);
	}

	case EExecutionApplyMode::Reserve:
	{
		return ReserveAction(InResult.ResolvedContext);
	}

	case EExecutionApplyMode::Intervene:
	{
		// [NOTE] Try Apply Intervention
		if (!ApplyExecutionInterventionDirective(InResult.InterventionDirective)) return false;
		if (!ApplyOverlayHandlings(InResult.OverlayHandlings)) return false;

		return StartAction(InResult.ResolvedContext);
	}

	default:
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
	if (InNotifyCommand == EActionNotifyCommand::None || InNotifyCommand == EActionNotifyCommand::Max) return;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyCommand(InNotifyCommand);
}

void UCActionComponent::HandleActionAllowInterventionWindowBegin(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->OpenAllowInterventionWindow(InWindowKey);
}

void UCActionComponent::HandleActionAllowInterventionWindowEnd(FName InWindowKey)
{
	if (InWindowKey.IsNone()) return;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->CloseAllowInterventionWindow(InWindowKey);
}

void UCActionComponent::HandleActionFeedback(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerOnce, InTriggerKey);
}

void UCActionComponent::HandleActionFeedbackWindowBegin(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerWindowBegin, InTriggerKey);
}

void UCActionComponent::HandleActionFeedbackWindowEnd(FName InTriggerKey)
{
	if (InTriggerKey.IsNone()) return;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerWindowEnd, InTriggerKey);
}

bool UCActionComponent::HandleActionCombatSignalCue(FName InCueTag)
{
	if (InCueTag.IsNone()) return false;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return false;

	FActionCombatSignalCueRequest request;
	if (!activeExecutor->ResolveNotifyCombatSignalCue(InCueTag, request)) return false;
	if (!request.IsValidRequest()) return false;

	if (!IsValid(CombatSignalSourceComp_Injected)) return false;
	return CombatSignalSourceComp_Injected->RequestAICombatSignalCue(request.CueTag);
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

	// bRebuildAll == true: Rebuild 
	// bRebuildAll == false: Append

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
				// [Debug] Duplicate key: Override data
				FLog::Log(TEXT("[Duplicate key] Overwrite Value"));
				ActionDataMap[actionDataKey] = actionData;
			}
			else // bRebuildAll == false
			{
				// [Policy] Currently set to 'skip'. (Options: ignore | restart | stop-then-play)
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

	// bRebuildAll == true: Rebuild 
	// bRebuildAll == false: Append

	if (bRebuildAll)
	{
		ActionExecutorMap.Reset();
	}

	for (const FActionData& actionData : ActionDatas)
	{
		if (!actionData.IsValidMinimal()) continue;

		UClass* executorkey = actionData.ActionExecutorKey.Get();
		if (!IsValid(executorkey)) continue;

		// 1) Find existing cached Reaction
		if (!bRebuildAll)
		{
			const UCAction* found = FindActionExecutor(executorkey);
			if (IsValid(found)) continue;
		}

		// 2) Add cached Reaction
		UCAction* add = AddActionExecutor(executorkey);
		if (!IsValid(add))
		{
			FLog::Log(FString::Printf(TEXT("[BuildActionExecutorMap] Failed to add ActionExecutor. ActionExecutorKey = %s"), *GetNameSafe(actionData.ActionExecutorKey.Get())));
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
		// Remove Invalid Entry
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
	if (IsActive()) return false;
	if (!InContext.IsValidMinimal()) return false;

	UCAction* incomingExecutor = InContext.ActionExecutor;
	if (!IsValid(incomingExecutor)) return false;

	const FActionData& incomingData = InContext.ActionData;

	EnterActionState(incomingData);

	if (!incomingExecutor->Start(incomingData))
	{
		ExitActionState(incomingData);
		return false;
	}

	SetActiveActionContext(InContext);
	return true;
}

bool UCActionComponent::ReserveAction(const FActionExecutionContext& InContext)
{
	if (!IsActive()) return false;
	if (!InContext.IsValidMinimal()) return false;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return false;

	const FActionData& incomingData = InContext.ActionData;

	return activeExecutor->ReserveChain(incomingData);
}

bool UCActionComponent::InterruptActiveAction(const FExecutionInterventionDirective& InDirective)
{
	if (!IsActive()) return true;

	const EActionFinishReason finishReason = ConvertExecutionStopReasonToActionFinishReason(InDirective.StopReason);

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		// [NOTE] Fallback
		return EndActiveAction(finishReason);
	}

	activeExecutor->Interrupt(InDirective);

	if (IsActive())
	{
		// [NOTE] Fallback
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
	const bool bAlive = IsValid(HealthComp_Injected) && HealthComp_Injected->IsAlive();
	const bool bDeadExecution = IsValid(StateComp_Injected) && StateComp_Injected->GetCurrentExecutionState() == EExecutionState::Dead;

	if (!bAlive || bDeadExecution) return;

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
