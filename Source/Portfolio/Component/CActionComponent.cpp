#include "Component/CActionComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CStateComponent.h"
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

	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	check(StateComp_Cached);

	for (const FActionDefinition& actionDefinition : ActionDefinitions)
	{
		if (!CreateAction(OwnerCharacter_Cached, actionDefinition))
		{
			FLog::Log(FString::Printf(TEXT("[ActionComponent] CreateAction failed. ActionType = %d"), (int32)actionDefinition.ActionType));

			continue;
		}
	}

	CurrentActionType = EActionType::Idle;
}

void UCActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TPair<EActionType, UCAction*>& Pair : ActionContainer)
	{
		UCAction* Action = Pair.Value;

		if (!IsValid(Action)) continue;

		Action->Tick(DeltaTime);
	}
}

UCAction* UCActionComponent::GetCurrentAction() const
{
	auto currentActionPtr = ActionContainer.Find(CurrentActionType);
	if (!currentActionPtr) return nullptr;

	UCAction* currentAction = *currentActionPtr;
	if (!IsValid(currentAction)) return nullptr;

	return currentAction;
}

FActionExecutionResult UCActionComponent::ExecuteAction(EActionType IncomingActionType)
{
	if (!IsValid(OwnerCharacter_Cached))
		return BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);

	UCAction** actionPtr = ActionContainer.Find(IncomingActionType);
	if (actionPtr == nullptr)
		return BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);

	UCAction* incomingAction = *actionPtr;
	if (!IsValid(incomingAction))
		return BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);

	const FActionExecutionQuery actionExecutionQuery = BuildActionExecutionQuery(IncomingActionType, incomingAction);
	const EActionExecutionDecision actionExecutionDecision = incomingAction->DecideExecution(actionExecutionQuery);

	switch (actionExecutionDecision)
	{
	case EActionExecutionDecision::Start:
	{
		return StartAction(incomingAction, IncomingActionType)
			? BuildActionExecutionResult(EActionExecutionDecision::Start, IncomingActionType)
			: BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);
	}

	case EActionExecutionDecision::Chain:
	{
		return ApplyActionChain(incomingAction, actionExecutionQuery)
			? BuildActionExecutionResult(EActionExecutionDecision::Chain, IncomingActionType)
			: BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);
	}

	case EActionExecutionDecision::Enqueue:
	{
		// TODO: Implement action enqueue.
		return BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);
	}

	case EActionExecutionDecision::Interrupt:
	{
		// TODO: Implement action interrupt.
		return BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);
	}

	case EActionExecutionDecision::Ignore:
	{
		return BuildActionExecutionResult(EActionExecutionDecision::Ignore, IncomingActionType);
	}

	case EActionExecutionDecision::Reject:
	default:
		return BuildActionExecutionResult(EActionExecutionDecision::Reject, IncomingActionType);
	}
}

void UCActionComponent::CompleteCurrentAction()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	UCAction* currentAction = GetCurrentAction();
	if (!IsValid(currentAction)) return;

	currentAction->Complete();

	ExitActionState();
}

void UCActionComponent::AbortCurrentAction(EActionAbortReason InActionAbortReason)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	UCAction* currentAction = GetCurrentAction();
	if (!IsValid(currentAction)) return;

	currentAction->Abort(InActionAbortReason);

	ExitActionState();
}

bool UCActionComponent::StartAction(UCAction* InAction, EActionType InActionType)
{
	if (!IsValid(InAction)) return false;

	EnterActionState(InActionType);

	if (!InAction->Start())
	{
		ExitActionState();
		return false;
	}

	return true;
}

bool UCActionComponent::ApplyActionChain(UCAction* InAction, const FActionExecutionQuery& InActionExecuteQuery)
{
	if (!IsValid(InAction)) return false;

	return InAction->ApplyChain(InActionExecuteQuery);
}

FActionExecutionQuery UCActionComponent::BuildActionExecutionQuery(EActionType InIncomingActionType, UCAction* InIncomingAction) const
{
	FActionExecutionQuery actionExecutionQuery;

	actionExecutionQuery.ExecutionState = IsValid(StateComp_Cached) ? StateComp_Cached->GetCurrentExecutionState() : EExecutionState::Dead;

	actionExecutionQuery.CurrentActionType = CurrentActionType;
	actionExecutionQuery.CurrentAction = GetCurrentAction();

	actionExecutionQuery.IncomingActionType = InIncomingActionType;
	actionExecutionQuery.IncomingAction = InIncomingAction;

	return actionExecutionQuery;
}

FActionExecutionResult UCActionComponent::BuildActionExecutionResult(EActionExecutionDecision InActionExecutionDecision, EActionType InActionType) const
{
	return FActionExecutionResult(InActionExecutionDecision, InActionType);
}

void UCActionComponent::EnterActionState(EActionType InActionType)
{
	if (!IsValid(StateComp_Cached)) return;

	StateComp_Cached->SetActionState();
	ChangeActionType(InActionType);
}

void UCActionComponent::ExitActionState()
{
	if (!IsValid(StateComp_Cached)) return;

	ChangeActionType(EActionType::Idle);
	StateComp_Cached->SetIdleState();
}

void UCActionComponent::ChangeActionType(EActionType InNewActionType)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	EActionType previousActionType = CurrentActionType;
	CurrentActionType = InNewActionType;

	if (OnActionTypeChanged.IsBound())
		OnActionTypeChanged.Broadcast(OwnerCharacter_Cached, previousActionType, CurrentActionType);
}

// InActionDefinition : ActionType / CAction Class / Datas
bool UCActionComponent::CreateAction(ACharacter* InOwnerCharacter, const FActionDefinition& InActionDefinition)
{
	if (!IsValid(InOwnerCharacter)) return false;

	if (InActionDefinition.ActionType == EActionType::Max)
	{
		FLog::Log(TEXT("[ActionComponent] ActionType is not set."));
		return false;
	}

	if (!IsValid(InActionDefinition.ActionClass))
	{
		FLog::Log(TEXT("[ActionComponent] ActionClass is not set."));
		return false;
	}

	if (ActionContainer.Contains(InActionDefinition.ActionType))
	{
		FLog::Log(FString::Printf(TEXT("[ActionComponent] ActionType already exists. ActionType = %d"), (int32)InActionDefinition.ActionType));
		return false;
	}

	UCAction* action = NewObject<UCAction>(this, InActionDefinition.ActionClass);

	if (!IsValid(action))
	{
		FLog::Log(TEXT("[ActionComponent]  Action was not created."));
		return false;
	}

	action->InitializeAction(InOwnerCharacter, InActionDefinition.ActionType, InActionDefinition.ActionDatas);

	ActionContainer.Add(InActionDefinition.ActionType, action);

	return true;
}