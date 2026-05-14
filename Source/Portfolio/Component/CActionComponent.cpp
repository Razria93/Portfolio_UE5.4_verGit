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

	if (!ApplyReactionStopDirective(InActionOrchestrationResult)) return false;

	switch (InActionOrchestrationResult.Decision)
	{
	case EActionOrchestrationLevelDecision::Start:
		return TryStartAction(InActionOrchestrationResult.ResolvedContext);

	case EActionOrchestrationLevelDecision::Chain:
		return TryChainAction(InActionOrchestrationResult.ResolvedContext);

	case EActionOrchestrationLevelDecision::Enqueue:
		return TryEnqueueAction(InActionOrchestrationResult.ResolvedContext);

	case EActionOrchestrationLevelDecision::Interrupt:
		return TryReplaceAction(InActionOrchestrationResult.ResolvedContext, EActionStopReason::Interrupted);

	case EActionOrchestrationLevelDecision::Cancel:
		return TryStartAction(InActionOrchestrationResult.ResolvedContext);

	default:
		return false;
	}
}

bool UCActionComponent::RequestStopActiveAction(EActionStopReason InActionStopReason)
{
	return TryStopActiveAction(InActionStopReason);
}

void UCActionComponent::HandleActionFinished(const UCAction* InAction, EActionFinishReason InActionFinishReason)
{
	if (!IsActive()) return;
	if (!IsValid(InAction)) return;
	if (InAction != GetActiveActionExecutor()) return;

	EndActiveActionInternal(InActionFinishReason);
}

void UCActionComponent::BroadcastActionEvent(EActionType InActionType, int32 InActionIndex, EActionEventType InActionEventType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	if (OnActionEvent.IsBound())
	{
		OnActionEvent.Broadcast(OwnerCharacter_Cached, InActionType, InActionIndex, InActionEventType);
	}
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

bool UCActionComponent::TryStartAction(const FActionResolvedContext& InActionResolvedContext)
{
	if (IsActive()) return false;
	if (!InActionResolvedContext.IsValidMinimal()) return false;

	return StartActiveActionInternal(InActionResolvedContext);
}

bool UCActionComponent::TryChainAction(const FActionResolvedContext& InActionResolvedContext)
{
	if (!IsActive()) return false;
	if (!InActionResolvedContext.IsValidMinimal()) return false;

	return ChainActiveActionInternal(InActionResolvedContext);
}

bool UCActionComponent::TryEnqueueAction(const FActionResolvedContext& InActionResolvedContext)
{
	return false;
}

bool UCActionComponent::TryReplaceAction(const FActionResolvedContext& InActionResolvedContext, EActionStopReason InStopReason)
{
	if (!IsActive()) return false;
	if (!InActionResolvedContext.IsValidMinimal()) return false;

	if (!StopActiveActionInternal(InStopReason)) return false;

	return StartActiveActionInternal(InActionResolvedContext);
}

bool UCActionComponent::TryStopActiveAction(EActionStopReason InStopReason)
{
	if (!IsActive()) return true;

	if (!StopActiveActionInternal(InStopReason)) return false;

	const EActionFinishReason finishReason = ConvertStopReasonToFinishReason(InStopReason);

	return EndActiveActionInternal(finishReason);
}

bool UCActionComponent::StartActiveActionInternal(const FActionResolvedContext& InActionResolvedContext)
{
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

bool UCActionComponent::ChainActiveActionInternal(const FActionResolvedContext& InActionResolvedContext)
{
	UCAction* actionExecutor = InActionResolvedContext.ActionExecutor;
	if (!IsValid(actionExecutor)) return false;

	const FActionData& incomingActionData = InActionResolvedContext.ActionData;

	return actionExecutor->ApplyChain(incomingActionData);
}

bool UCActionComponent::StopActiveActionInternal(EActionStopReason InStopReason)
{
	if (!IsActive()) return true;

	UCAction* actionExecutor = GetActiveActionExecutor();
	if (!IsValid(actionExecutor))
	{
		// Stale Guard
		const EActionFinishReason finishReason = ConvertStopReasonToFinishReason(InStopReason);
		EndActiveActionInternal(finishReason);

		return !IsActive();
	}

	actionExecutor->Stop(InStopReason);

	return !IsActive();
}

bool UCActionComponent::EndActiveActionInternal(EActionFinishReason InFinishReason)
{
	if (!IsActive()) return true;

	// None Reference. Snapshot before clearing active context.
	const FActionData activeData = ActiveActionData;

	if (activeData.IsValidMinimal())
	{
		ExitActionState(activeData);
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

bool UCActionComponent::HandleActionChained(const UCAction* InAction, const FActionData& InActionData)
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

bool UCActionComponent::ApplyReactionStopDirective(const FActionOrchestrationLevelResult& InActionOrchestrationResult)
{
	if (!InActionOrchestrationResult.StopDirective.IsValidRequest()) return true;
	if (!IsValid(ReactionComp_Cached)) return true;

	const EReactionType activeReactionType = ReactionComp_Cached->GetActiveReactionType();

	if (activeReactionType == EReactionType::None
		|| activeReactionType == EReactionType::Idle
		|| activeReactionType == EReactionType::All
		|| activeReactionType == EReactionType::Max)
	{
		return true;
	}

	if (activeReactionType == EReactionType::Dead)
	{
		return false;
	}

	return ReactionComp_Cached->RequestStopActiveReaction(InActionOrchestrationResult.StopDirective);
}

EActionFinishReason UCActionComponent::ConvertStopReasonToFinishReason(EActionStopReason InStopReason) const
{
	switch (InStopReason)
	{
	case EActionStopReason::Interrupted:
		return EActionFinishReason::Interrupted;

	case EActionStopReason::Cancelled:
		return EActionFinishReason::Cancelled;

	case EActionStopReason::Ignored:
		return EActionFinishReason::Ignored;

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
