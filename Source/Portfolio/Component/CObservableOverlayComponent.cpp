#include "Component/CObservableOverlayComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

UCObservableOverlayComponent::UCObservableOverlayComponent()
{
}

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

bool UCObservableOverlayComponent::ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	for (const EObservableOverlayHandling handling : InHandlings)
	{
		if (!ApplyOverlayHandling(handling))
		{
			FLog::Log(FString::Printf(
				TEXT("[OverlayHandling] Failed Handling=%s"),
				*UEnum::GetValueAsString(handling)));
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

	FLog::Log(FString::Printf(
		TEXT("[OverlayHandling] No policy accepted Handling=%s"),
		*UEnum::GetValueAsString(InHandling)));
	return false;
}

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
