#include "Component/CStateComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Type/CStateStructure.h"
#include "Type/CHealthStructure.h"

UCStateComponent::UCStateComponent()
{
}

void UCStateComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);
}

// Sync Health 'dead-state' changes into the 'execution state'.
void UCStateComponent::OnDeadStateChanged(EDeadState InPrevDeadState, EDeadState InNewDeadState)
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	switch (InNewDeadState)
	{
	case EDeadState::Alive:
	{
		if (CheckCurExecutionState(EExecutionState::Dead))
		{
			// [Health: Alive] [Execution: Dead] -> restore Idle
			SetIdleState();
			break;
		}

		// [Health: Alive] [Execution: Non-Dead] -> keep current execution
		break;
	}

	// Dying / Dead / Reviving : Non-Alive
	case EDeadState::Dying:
	case EDeadState::Dead:
	case EDeadState::Reviving:
	{
		// [Health: Non-Alive] [Execution: Any] -> force Dead
		SetDeadState();
		break;
	}

	default:
		break;
	}
}

void UCStateComponent::SetIdleState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeExecutionState(EExecutionState::Idle);
}

void UCStateComponent::SetActionState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeExecutionState(EExecutionState::Action);
}

void UCStateComponent::SetReactionState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeExecutionState(EExecutionState::Reaction);
}

void UCStateComponent::SetDeadState()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	ChangeExecutionState(EExecutionState::Dead);
}

void UCStateComponent::ChangeExecutionState(EExecutionState InNewExecutionState)
{
	if (!IsValid(OwnerCharacter_Cached)) return;
	if (CurrentExecutionState == InNewExecutionState) return;

	EExecutionState prevExecutionState = CurrentExecutionState;
	CurrentExecutionState = InNewExecutionState;

	PrintExecutionStateChangedInfo(prevExecutionState, CurrentExecutionState);

	if (OnExecutionStateChanged.IsBound())
	{
		OnExecutionStateChanged.Broadcast(OwnerCharacter_Cached, prevExecutionState, CurrentExecutionState);
	}
}

void UCStateComponent::PrintExecutionStateChangedInfo(EExecutionState InPrevExecutionState, EExecutionState InNewExecutionState) const
{
	FLog::Log(FString::Printf(
		TEXT("[ExecutionStateChanged] Owner = %s | PrevState = %s | NewState = %s"),
		*GetNameSafe(OwnerCharacter_Cached),
		*UEnum::GetValueAsString(InPrevExecutionState),
		*UEnum::GetValueAsString(InNewExecutionState)));
}