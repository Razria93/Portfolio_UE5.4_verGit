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
	auto curActionPtr = ActionContainer.Find(CurrentActionType);
	if (!curActionPtr) return nullptr;

	UCAction* curAction = *curActionPtr;
	if (!IsValid(curAction)) return nullptr;

	return curAction;
}

bool UCActionComponent::CanStartAction() const
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!IsValid(StateComp_Cached)) return false;

	if (!StateComp_Cached->CheckCurExecutionState(EExecutionState::Idle)) return false;
	if (CurrentActionType != EActionType::Idle) return false;

	return true;
}

bool UCActionComponent::StartAction(EActionType InActionType)
{
	if (!IsValid(OwnerCharacter_Cached)) return false;

	UCAction** actionPtr = ActionContainer.Find(InActionType);
	if (actionPtr == nullptr) return false;

	UCAction* action = *actionPtr;
	if (!IsValid(action)) return false;

	// [Exception] Combo Attack
	if (CurrentActionType == InActionType)
	{
		if (!IsValid(StateComp_Cached)) return false;
		if (!StateComp_Cached->CheckCurExecutionState(EExecutionState::Action)) return false;
		if (CurrentActionType != EActionType::ComboAttack || InActionType != EActionType::ComboAttack) return false;

		return action->Start();
	}

	if (!CanStartAction()) return false;

	EnterActionState(InActionType);

	if (!action->Start())
	{
		ExitActionState();
		return false;
	}

	return true;
}

void UCActionComponent::CompleteAction()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	UCAction* currentAction = GetCurrentAction();
	if (!IsValid(currentAction)) return;

	currentAction->Complete();

	ExitActionState();
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