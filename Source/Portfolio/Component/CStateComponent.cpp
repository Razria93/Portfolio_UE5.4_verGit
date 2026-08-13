#include "Component/CStateComponent.h"

#include "ProjectGlobal.h"

#include "Type/CStateTypes.h"

#include "GameFramework/Character.h"

UCStateComponent::UCStateComponent()
{
}

// Component Reference
void UCStateComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;

	ValidateRequiredComponentReferences();
}

bool UCStateComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

// Mutation
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

void UCStateComponent::ChangeExecutionState(EExecutionState InNewExecutionState)
{
	if (!IsValid(OwnerCharacter_Injected)) return;
	if (CurrentExecutionState == InNewExecutionState) return;

	EExecutionState prevExecutionState = CurrentExecutionState;
	CurrentExecutionState = InNewExecutionState;

	if (OnExecutionStateChanged.IsBound())
	{
		OnExecutionStateChanged.Broadcast(OwnerCharacter_Injected, prevExecutionState, CurrentExecutionState);
	}
}
