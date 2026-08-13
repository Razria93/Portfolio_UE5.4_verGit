#include "Component/CCharacterFeedbackComponent.h"

#include "ProjectGlobal.h"

#include "Core/Debug/FDeathLifecycleDebug.h"

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

bool UCCharacterFeedbackComponent::RequestDeathPresentation(EDeathPresentationReason InReason)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (InReason == EDeathPresentationReason::None || InReason == EDeathPresentationReason::Max) return false;

	if (DeathPresentationState != EDeathPresentationRuntimeState::Inactive)
	{
		return true;
	}

	if (!OnDeathPresentationRequested.IsBound()) return false;

	DeathPresentationState = EDeathPresentationRuntimeState::Requested;

	OnDeathPresentationRequested.Broadcast(InReason);
	return true;
}

void UCCharacterFeedbackComponent::NotifyDeathPresentationStarted()
{
	if (DeathPresentationState != EDeathPresentationRuntimeState::Requested)
	{
		FDeathLifecycleDebug::RecordContractViolationForAudit(OwnerCharacter_Injected, TEXT("PresentationNotifyIgnored"), FString::Printf(TEXT("Event: Started | State: %s"), *UEnum::GetValueAsString(DeathPresentationState)));
		return;
	}

	DeathPresentationState = EDeathPresentationRuntimeState::Active;
	OnDeathPresentationEvent.Broadcast(EDeathPresentationEventType::Started);
}

void UCCharacterFeedbackComponent::NotifyDeathPresentationUnavailable()
{
	if (DeathPresentationState != EDeathPresentationRuntimeState::Requested)
	{
		FDeathLifecycleDebug::RecordContractViolationForAudit(OwnerCharacter_Injected, TEXT("PresentationNotifyIgnored"), FString::Printf(TEXT("Event: Unavailable | State: %s"), *UEnum::GetValueAsString(DeathPresentationState)));
		return;
	}

	DeathPresentationState = EDeathPresentationRuntimeState::Inactive;
	OnDeathPresentationEvent.Broadcast(EDeathPresentationEventType::Unavailable);
}

void UCCharacterFeedbackComponent::NotifyDeathPresentationFinished()
{
	if (DeathPresentationState != EDeathPresentationRuntimeState::Active)
	{
		FDeathLifecycleDebug::RecordContractViolationForAudit(OwnerCharacter_Injected, TEXT("PresentationNotifyIgnored"), FString::Printf(TEXT("Event: Finished | State: %s"), *UEnum::GetValueAsString(DeathPresentationState)));
		return;
	}

	DeathPresentationState = EDeathPresentationRuntimeState::Inactive;
	OnDeathPresentationEvent.Broadcast(EDeathPresentationEventType::Finished);
}

void UCCharacterFeedbackComponent::ClearRuntimeFeedback()
{
	DeathPresentationState = EDeathPresentationRuntimeState::Inactive;
}
