#include "Component/CActionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Action/CAction.h"

#include "Type/CWeaponStructure.h"

UCActionComponent::UCActionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCActionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	MovementComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCMovementComponent>();
	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	ActionOrchestratorComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCActionOrchestratorComponent>();
	DefenseComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCDefenseComponent>();
	ReactionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCReactionComponent>();
	ObservableOverlayComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCObservableOverlayComponent>();

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

bool UCActionComponent::CanCommitChain(const UCAction* InAction, const FActionData& InData) const
{
	if (!IsActive()) return false;

	if (!IsValid(InAction)) return false;
	if (InAction != GetActiveActionExecutor()) return false;
	if (!InData.IsValidMinimal()) return false;

	if (!IsValid(HealthComp_Cached) || !HealthComp_Cached->IsAlive()) return false;
	
	if (!IsValid(StateComp_Cached)) return false;
	if (StateComp_Cached->GetCurrentExecutionState() != EExecutionState::Action) return false;

	return true;
}

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

bool UCActionComponent::ApplyActionDecision(const FActionExecutionResult& InResult)
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!InResult.IsAcceptedDecision()) return false;

	switch (InResult.ApplyMode)
	{
	case EExecutionApplyMode::Start:
	{
		if (!ApplyObservableOverlayHandlings(InResult.OverlayHandlings)) return false;

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
		if (!ApplyObservableOverlayHandlings(InResult.OverlayHandlings)) return false;

		return StartAction(InResult.ResolvedContext);
	}

	default:
		return false;
	}
}

bool UCActionComponent::RequestStopActiveAction(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsValidRequest()) return false;
	if (InDirective.TargetDomain != EExecutionDomain::Action) return false;

	return StopActiveAction(InDirective);
}

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

void UCActionComponent::BroadcastActionEvent(EActionType InType, int32 InIndex, EActionEventType InEventType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	if (OnActionEvent.IsBound())
	{
		OnActionEvent.Broadcast(OwnerCharacter_Cached, InType, InIndex, InEventType);
	}
}

void UCActionComponent::NotifyGuardInStarted()
{
	if (!IsValid(DefenseComp_Cached)) return;

	DefenseComp_Cached->HandleGuardInStarted();
}

void UCActionComponent::NotifyGuardOutStarted()
{
	if (!IsValid(DefenseComp_Cached)) return;

	DefenseComp_Cached->HandleGuardOutStarted();
}

void UCActionComponent::NotifyGuardInCompleted()
{
	if (!IsValid(ActionOrchestratorComp_Cached)) return;

	ActionOrchestratorComp_Cached->ConsumeDeferredAction(EDeferredActionConsumeKey::GuardInCompleted);
}

void UCActionComponent::NotifyGuardOutCompleted()
{
	if (!IsValid(DefenseComp_Cached)) return;

	DefenseComp_Cached->HandleGuardOutCompleted();
}

void UCActionComponent::NotifyGuardInterrupted(EActionStopReason InStopReason)
{
	if (IsValid(ActionOrchestratorComp_Cached))
	{
		ActionOrchestratorComp_Cached->ClearDeferredActions(EDeferredActionConsumeKey::GuardInCompleted);
	}

	if (!IsValid(DefenseComp_Cached)) return;

	DefenseComp_Cached->HandleGuardInterrupted(InStopReason);
}

void UCActionComponent::BuildActionDataMap(bool bRebuildAll)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

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
	if (!IsValid(OwnerCharacter_Cached)) return;

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

UCAction* UCActionComponent::AddActionExecutor(const TSubclassOf<class UCAction> InSubClass)
{
	UClass* executorKey = InSubClass.Get();
	if (!IsValid(executorKey)) return nullptr;

	UCAction* add = NewObject<UCAction>(this, InSubClass);
	if (!IsValid(add)) return nullptr;

	add->InitializeAction(OwnerCharacter_Cached, this);
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

bool UCActionComponent::ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InDirective)
{
	if (!InDirective.IsRequested()) return true;
	if (!InDirective.IsValidRequest()) return false;

	switch (InDirective.TargetDomain)
	{
	case EExecutionDomain::Action:
		return StopActiveAction(InDirective);

	case EExecutionDomain::Reaction:
		return IsValid(ReactionComp_Cached) && ReactionComp_Cached->RequestStopActiveReaction(InDirective);

	default:
		return false;
	}
}

bool UCActionComponent::ApplyObservableOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	if (InHandlings.IsEmpty()) return true;
	return IsValid(ObservableOverlayComp_Cached) && ObservableOverlayComp_Cached->ApplyObservableOverlayHandlings(InHandlings);
}

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

bool UCActionComponent::StopActiveAction(const FExecutionInterventionDirective& InDirective)
{
	if (!IsActive()) return true;

	const EActionStopReason stopReason = ConvertExecutionStopReasonToActionStopReason(InDirective.StopReason);
	const EActionFinishReason finishReason = ConvertExecutionStopReasonToActionFinishReason(InDirective.StopReason);

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor))
	{
		// [NOTE] Fallback
		return EndActiveAction(finishReason);
	}

	activeExecutor->Stop(stopReason);

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
		OnActionTypeChanged.Broadcast(OwnerCharacter_Cached, prevActionType, ActiveActionType);
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
		OnActionTypeChanged.Broadcast(OwnerCharacter_Cached, prevActionType, ActiveActionType);
	}
}

void UCActionComponent::EnterActionState(const FActionData& InData)
{
	if (IsValid(MovementComp_Cached) && !InData.bCanMove)
	{
		MovementComp_Cached->SetStop();
	}

	if (IsValid(StateComp_Cached))
	{
		StateComp_Cached->SetActionState();
	}
}

void UCActionComponent::ExitActionState(const FActionData& InData)
{
	const bool bAlive = IsValid(HealthComp_Cached) && HealthComp_Cached->IsAlive();
	const bool bDeadExecution = IsValid(StateComp_Cached) && StateComp_Cached->GetCurrentExecutionState() == EExecutionState::Dead;

	if (!bAlive || bDeadExecution) return;

	if (IsValid(MovementComp_Cached) && !InData.bCanMove)
	{
		MovementComp_Cached->SetMove();
	}

	if (IsValid(StateComp_Cached))
	{
		StateComp_Cached->SetIdleState();
	}
}

EActionStopReason UCActionComponent::ConvertExecutionStopReasonToActionStopReason(EExecutionStopReason InStopReason) const
{
	switch (InStopReason)
	{
	case EExecutionStopReason::Interrupted:
		return EActionStopReason::Interrupted;

	default:
		return EActionStopReason::Ignored;
	}
}

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
