#include "Component/CObservableOverlayComponent.h"

#include "ProjectGlobal.h"

#include "Core/Debug/FObservableOverlayDebug.h"

#include "GameFramework/Character.h"

UCObservableOverlayComponent::UCObservableOverlayComponent()
{
}

// Component Reference

void UCObservableOverlayComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	ValidateRequiredComponentReferences();

	MarkPolicyRegistryDirty();
	RefreshPolicyRegistry();
}

bool UCObservableOverlayComponent::ValidateRequiredComponentReferences() const
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

// Overlay Snapshot

void UCObservableOverlayComponent::WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot)
{
	RefreshPolicyRegistry();

	for (const TScriptInterface<IObservableOverlayPolicy>& policy : ObservableOverlayPolicies)
	{
		const IObservableOverlayPolicy* overlayPolicy = policy.GetInterface();
		if (!overlayPolicy) continue;

		// Each overlay policy writes its runtime state into the shared snapshot.
		overlayPolicy->WriteOverlaySnapshot(OutSnapshot);
	}
}

// Overlay Event

bool UCObservableOverlayComponent::ApplyOverlayEvent(const FObservableOverlayEventContext& InContext)
{
	if (!InContext.IsValidMinimal()) return false;

	RefreshPolicyRegistry();

	for (const TScriptInterface<IObservableOverlayPolicy>& policy : ObservableOverlayPolicies)
	{
		IObservableOverlayPolicy* overlayPolicy = policy.GetInterface();
		if (!overlayPolicy) continue;
		if (!overlayPolicy->CanApplyOverlayEvent(InContext)) continue;

		return overlayPolicy->ApplyOverlayEvent(InContext);
	}

	return false;
}

// Overlay Handling

bool UCObservableOverlayComponent::ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	for (const EObservableOverlayHandling handling : InHandlings)
	{
		if (!ApplyOverlayHandling(handling))
		{
			FObservableOverlayDebug::RecordOverlayHandlingsRejectedForAudit(OwnerCharacter_Injected, this, InHandlings, handling, TEXT("HandlingRejected"));
			return false;
		}
	}

	return true;
}

bool UCObservableOverlayComponent::ApplyOverlayHandling(EObservableOverlayHandling InHandling)
{
	if (InHandling == EObservableOverlayHandling::None) return true;

	RefreshPolicyRegistry();

	for (const TScriptInterface<IObservableOverlayPolicy>& policy : ObservableOverlayPolicies)
	{
		IObservableOverlayPolicy* overlayPolicy = policy.GetInterface();
		if (!overlayPolicy) continue;

		if (!overlayPolicy->CanApplyOverlayHandling(InHandling)) continue;

		return overlayPolicy->ApplyOverlayHandling(InHandling);
	}

	FObservableOverlayDebug::RecordOverlayHandlingRejectedForAudit(OwnerCharacter_Injected, this, InHandling, TEXT("NoPolicyAccepted"));
	return false;
}

// Policy Registry

void UCObservableOverlayComponent::MarkPolicyRegistryDirty()
{
	bOverlayPolicyRegistryDirty = true;
}

void UCObservableOverlayComponent::RefreshPolicyRegistry()
{
	if (!bOverlayPolicyRegistryDirty) return;

	RebuildPolicyRegistry();
}

void UCObservableOverlayComponent::RebuildPolicyRegistry()
{
	ObservableOverlayPolicies.Empty();

	TArray<UActorComponent*> ownerComponents;
	if (!IsValid(OwnerCharacter_Injected)) return;

	OwnerCharacter_Injected->GetComponents(ownerComponents);

	for (UActorComponent* component : ownerComponents)
	{
		if (!IsValid(component)) continue;
		if (!component->GetClass()->ImplementsInterface(UObservableOverlayPolicy::StaticClass())) continue;

		// UINTERFACE wrapper: keeps the UObject and interface pointer together.
		TScriptInterface<IObservableOverlayPolicy> policy;
		policy.SetObject(component);
		policy.SetInterface(Cast<IObservableOverlayPolicy>(component));

		if (policy.GetInterface())
		{
			ObservableOverlayPolicies.Add(policy);
		}
	}

	bOverlayPolicyRegistryDirty = false;
}
