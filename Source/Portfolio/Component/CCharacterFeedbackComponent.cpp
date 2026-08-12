#include "Component/CCharacterFeedbackComponent.h"

#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

UCCharacterFeedbackComponent::UCCharacterFeedbackComponent()
{
}

// Component Reference

void UCCharacterFeedbackComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	ValidateRequiredComponentReferences();
}

bool UCCharacterFeedbackComponent::ValidateRequiredComponentReferences() const
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

// Death Presentation

FDeathPresentationStartResult UCCharacterFeedbackComponent::StartDeathPresentation(EDeathPresentationReason InReason)
{
	FDeathPresentationStartResult result;
	result.ExpectedDuration = FMath::Max(DeathPresentationExpectedDuration, 0.f);

	if (!IsValid(OwnerCharacter_Injected)) return result;
	if (InReason == EDeathPresentationReason::None || InReason == EDeathPresentationReason::Max) return result;

	if (bDeathPresentationActive)
	{
		result.bStarted = true;
		return result;
	}

	if (!OnDeathPresentationRequested.IsBound()) return result;

	bDeathPresentationActive = true;

	result.bStarted = true;

	OnDeathPresentationRequested.Broadcast(InReason);
	return result;
}

void UCCharacterFeedbackComponent::NotifyDeathPresentationFinished()
{
	if (!bDeathPresentationActive) return;

	bDeathPresentationActive = false;
	OnDeathPresentationFinished.Broadcast();
}

void UCCharacterFeedbackComponent::ClearRuntimeFeedback()
{
	bDeathPresentationActive = false;
}
