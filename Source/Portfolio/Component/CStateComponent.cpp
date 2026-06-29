#include "Component/CStateComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Type/CStateStructure.h"
#include "Type/CHealthStructure.h"

UCStateComponent::UCStateComponent()
{
}

void UCStateComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;

	ValidateRequiredComponentReferences();
}

bool UCStateComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	struct FRequiredComponentReference
	{
		const UObject* Object = nullptr;
		const TCHAR* Label = TEXT("");
	};

	const FRequiredComponentReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
	};

	for (const FRequiredComponentReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Sync Health 'dead-state' changes into the 'execution state'.
void UCStateComponent::OnDeadStateChanged(EDeadState InPrevDeadState, EDeadState InNewDeadState)
{
	if (!IsValid(OwnerCharacter_Injected)) return;

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
	if (!IsValid(OwnerCharacter_Injected)) return;

	ChangeExecutionState(EExecutionState::Idle);
}

void UCStateComponent::SetActionState()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	ChangeExecutionState(EExecutionState::Action);
}

void UCStateComponent::SetReactionState()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	ChangeExecutionState(EExecutionState::Reaction);
}

void UCStateComponent::SetDeadState()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	ChangeExecutionState(EExecutionState::Dead);
}

void UCStateComponent::ChangeExecutionState(EExecutionState InNewExecutionState)
{
	if (!IsValid(OwnerCharacter_Injected)) return;
	if (CurrentExecutionState == InNewExecutionState) return;

	EExecutionState prevExecutionState = CurrentExecutionState;
	CurrentExecutionState = InNewExecutionState;

	// PrintExecutionStateChangedInfo(prevExecutionState, CurrentExecutionState);

	if (OnExecutionStateChanged.IsBound())
	{
		OnExecutionStateChanged.Broadcast(OwnerCharacter_Injected, prevExecutionState, CurrentExecutionState);
	}
}

void UCStateComponent::PrintExecutionStateChangedInfo(EExecutionState InPrevExecutionState, EExecutionState InNewExecutionState) const
{
	FLog::Log(FString::Printf(
		TEXT("[ExecutionStateChanged] Owner = %s | PrevState = %s | NewState = %s"),
		*GetNameSafe(OwnerCharacter_Injected),
		*UEnum::GetValueAsString(InPrevExecutionState),
		*UEnum::GetValueAsString(InNewExecutionState)));
}
