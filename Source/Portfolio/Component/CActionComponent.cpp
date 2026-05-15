#include "Component/CActionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CHealthComponent.h"
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
	ReactionComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCReactionComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();

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

bool UCActionComponent::GetActiveActionData(FActionData& OutActionData) const
{
	OutActionData = FActionData();

	if (!IsActive()) return false;
	if (!ActiveActionData.IsValidMinimal()) return false;

	OutActionData = ActiveActionData;
	return true;
}

UCAction* UCActionComponent::GetActiveActionExecutor() const
{
	if (!IsActive()) return nullptr;
	if (!IsValid(ActiveActionExecutor)) return nullptr;

	return ActiveActionExecutor;
}

bool UCActionComponent::ResolveActionData(const FActionDataKey& InActionDataKey, FActionData& OutActionData)
{
	OutActionData = FActionData();

	if (!InActionDataKey.IsValidExactKey()) return false;

	FActionData const* foundPtr = ActionDataMap.Find(InActionDataKey);
	if (foundPtr == nullptr) return false;

	FActionData found = *foundPtr;

	OutActionData = found;

	return found.IsValidMinimal();
}

UCAction* UCActionComponent::ResolveActionExecutor(const FActionData& InActionData)
{
	// 1) Try reuse cached Reaction; return if valid
	UCAction* foundAction = FindActionExecutor(InActionData.ActionExecutorKey.Get());
	if (IsValid(foundAction)) return foundAction;

	// 2) [Policy] Try Add and cache Reaction; return if valid
	UCAction* addAction = AddActionExecutor(InActionData.ActionExecutorKey);
	if (IsValid(addAction)) return addAction;

	// [Debug] ReactionData is Valid; but Find and Add Failed
	return nullptr;
}

bool UCActionComponent::ApplyActionDecision(const FActionOrchestrationLevelResult& InActionOrchestrationResult)
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!InActionOrchestrationResult.IsAcceptedDecision()) return false;

	if (!ApplyExecutionInterventionDirective(InActionOrchestrationResult.InterventionDirective)) return false;

	switch (InActionOrchestrationResult.Decision)
	{
	case EActionOrchestrationLevelDecision::Start:
	case EActionOrchestrationLevelDecision::Interrupt:
	case EActionOrchestrationLevelDecision::Cancel:
		return StartAction(InActionOrchestrationResult.ResolvedContext);

	case EActionOrchestrationLevelDecision::Chain:
		return ChainActiveAction(InActionOrchestrationResult.ResolvedContext);

	default:
		return false;
	}
}

bool UCActionComponent::RequestStopActiveAction(const FExecutionInterventionDirective& InInterventionDirective)
{
	if (!InInterventionDirective.IsValidRequest()) return false;
	if (InInterventionDirective.TargetDomain != EExecutionDomain::Action) return false;

	return StopActiveAction(InInterventionDirective);
}

bool UCActionComponent::HandleApplyActionChained(const UCAction* InAction, const FActionData& InActionData)
{
	if (!IsActive()) return false;
	if (!IsValid(InAction)) return false;
	if (InAction != GetActiveActionExecutor()) return false;
	if (!InActionData.IsValidMinimal()) return false;

	ActiveActionType = InActionData.ActionDataKey.ActionType;
	ActiveActionIndex = InActionData.ActionDataKey.ActionIndex;
	ActiveActionData = InActionData;

	return true;
}

void UCActionComponent::HandleApplyActionFinished(const UCAction* InAction, EActionFinishReason InActionFinishReason)
{
	if (!IsActive()) return;
	if (!IsValid(InAction)) return;
	if (InAction != GetActiveActionExecutor()) return;

	EndActiveAction(InActionFinishReason);
}

void UCActionComponent::HandleActionNotifyCommand(EActionNotifyCommand InNotifyCommand)
{
	if (InNotifyCommand == EActionNotifyCommand::None || InNotifyCommand == EActionNotifyCommand::Max) return;

	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyCommand(InNotifyCommand);
}

void UCActionComponent::HandleActionFeedback(FName InTriggerKey)
{
	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerOnce, InTriggerKey);
}

void UCActionComponent::HandleActionFeedbackWindowBegin(FName InTriggerKey)
{
	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerWindowBegin, InTriggerKey);
}

void UCActionComponent::HandleActionFeedbackWindowEnd(FName InTriggerKey)
{
	UCAction* activeExecutor = GetActiveActionExecutor();
	if (!IsValid(activeExecutor)) return;

	activeExecutor->HandleNotifyFeedback(EActionFeedbackTiming::TriggerWindowEnd, InTriggerKey);
}

void UCActionComponent::BroadcastActionEvent(EActionType InActionType, int32 InActionIndex, EActionEventType InActionEventType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	if (OnActionEvent.IsBound())
	{
		OnActionEvent.Broadcast(OwnerCharacter_Cached, InActionType, InActionIndex, InActionEventType);
	}
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
		if (!actionDataKey.IsValidExactKey()) continue;

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

		UClass* keyClass = actionData.ActionExecutorKey.Get();
		if (!IsValid(keyClass)) continue;

		// 1) Find existing cached Reaction
		if (!bRebuildAll)
		{
			const UCAction* found = FindActionExecutor(keyClass);
			if (IsValid(found)) continue;
		}

		// 2) Add cached Reaction
		UCAction* add = AddActionExecutor(keyClass);
		if (!IsValid(add))
		{
			FLog::Log(FString::Printf(TEXT("[BuildActionExecutorMap] Failed to add ActionExecutor. ActionExecutorKey = %s"), *GetNameSafe(actionData.ActionExecutorKey.Get())));
			continue;
		}
	}
}

UCAction* UCActionComponent::AddActionExecutor(const TSubclassOf<class UCAction> InSubClass)
{
	UClass* keyClass = InSubClass.Get();
	if (!IsValid(keyClass)) return nullptr;

	UCAction* add = NewObject<UCAction>(this, InSubClass);
	if (!IsValid(add)) return nullptr;

	add->InitializeAction(OwnerCharacter_Cached, this);
	ActionExecutorMap.Add(keyClass, add);

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

bool UCActionComponent::ApplyExecutionInterventionDirective(const FExecutionInterventionDirective& InInterventionDirective)
{
	if (!InInterventionDirective.IsRequested()) return true;
	if (!InInterventionDirective.IsValidRequest()) return true;

	const EReactionType activeReactionType = ReactionComp_Cached->GetActiveReactionType();

	switch (InInterventionDirective.TargetDomain)
	{
	case EExecutionDomain::Action:
		return StopActiveAction(InInterventionDirective);

	case EExecutionDomain::Reaction:
		return IsValid(ReactionComp_Cached) && ReactionComp_Cached->RequestStopActiveReaction(InInterventionDirective);

	default:
		return false;
	}
}

bool UCActionComponent::StartAction(const FActionResolvedContext& InActionResolvedContext)
{
	if (IsActive()) return false;
	if (!InActionResolvedContext.IsValidMinimal()) return false;

	UCAction* actionExecutor = InActionResolvedContext.ActionExecutor;
	if (!IsValid(actionExecutor)) return false;

	const FActionData& incomingActionData = InActionResolvedContext.ActionData;

	EnterActionState(incomingActionData);

	if (!actionExecutor->Start(incomingActionData))
	{
		ExitActionState(incomingActionData);
		return false;
	}

	SetActiveActionContext(InActionResolvedContext);
	return true;
}

bool UCActionComponent::ChainActiveAction(const FActionResolvedContext& InActionResolvedContext)
{
	if (!IsActive()) return false;
	if (!InActionResolvedContext.IsValidMinimal()) return false;

	UCAction* actionExecutor = InActionResolvedContext.ActionExecutor;
	if (!IsValid(actionExecutor)) return false;

	const FActionData& incomingActionData = InActionResolvedContext.ActionData;

	return actionExecutor->ApplyChain(incomingActionData);
}

bool UCActionComponent::StopActiveAction(const FExecutionInterventionDirective& InInterventionDirective)
{
	if (!IsActive()) return true;

	const EActionStopReason stopReason = ConvertExecutionStopReasonToActionStopReason(InInterventionDirective.StopReason);
	const EActionFinishReason finishReason = ConvertExecutionStopReasonToActionFinishReason(InInterventionDirective.StopReason);

	UCAction* actionExecutor = GetActiveActionExecutor();
	if (!IsValid(actionExecutor))
	{
		// [NOTE] Fallback
		return EndActiveAction(finishReason);
	}

	actionExecutor->Stop(stopReason);

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

	const FActionData activeActionData = ActiveActionData;

	if (activeActionData.IsValidMinimal())
	{
		ExitActionState(activeActionData);
	}

	ClearActiveActionContext();

	return !IsActive();
}

void UCActionComponent::SetActiveActionContext(const FActionResolvedContext& InActionResolvedContext)
{
	if (!InActionResolvedContext.IsValidMinimal()) return;

	const EActionType prevActionType = ActiveActionType;

	ActiveActionType = InActionResolvedContext.ActionData.ActionDataKey.ActionType;
	ActiveActionIndex = InActionResolvedContext.ActionData.ActionDataKey.ActionIndex;
	ActiveActionData = InActionResolvedContext.ActionData;
	ActiveActionExecutor = InActionResolvedContext.ActionExecutor;

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

void UCActionComponent::EnterActionState(const FActionData& InActionData)
{
	if (IsValid(MovementComp_Cached) && !InActionData.bCanMove)
	{
		MovementComp_Cached->SetStop();
	}

	if (IsValid(StateComp_Cached))
	{
		StateComp_Cached->SetActionState();
	}
}

void UCActionComponent::ExitActionState(const FActionData& InActionData)
{
	const bool bAlive = IsValid(HealthComp_Cached) && HealthComp_Cached->IsAlive();
	const bool bDeadExecution = IsValid(StateComp_Cached) && StateComp_Cached->GetCurrentExecutionState() == EExecutionState::Dead;

	if (!bAlive || bDeadExecution) return;

	if (IsValid(MovementComp_Cached) && !InActionData.bCanMove)
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

	case EExecutionStopReason::Cancelled:
		return EActionStopReason::Cancelled;

	case EExecutionStopReason::Ignored:
		return EActionStopReason::Ignored;

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

	case EExecutionStopReason::Cancelled:
		return EActionFinishReason::Cancelled;

	case EExecutionStopReason::Ignored:
	default:
		return EActionFinishReason::Ignored;
	}
}

void UCActionComponent::PrintActionLocalLevelQuery(const FActionLocalLevelQuery& InQuery) const
{
	FLog::Log(TEXT("==== ActionLocalLevelQuery ===="));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ExecutionState"), *UEnum::GetValueAsString(InQuery.ExecutionState)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActiveActionType"), *UEnum::GetValueAsString(InQuery.ActiveContext.ActionDataKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("ActiveActionExecutor"), *GetNameSafe(InQuery.ActiveContext.ActionExecutor)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("IncomingActionType"), *UEnum::GetValueAsString(InQuery.IncomingContext.ActionDataKey.ActionType)));
	FLog::Log(FString::Printf(TEXT("%-20s: %s"), TEXT("IncomingActionExecutor"), *GetNameSafe(InQuery.IncomingContext.ActionExecutor)));
	FLog::Log(TEXT("================================"));
}
