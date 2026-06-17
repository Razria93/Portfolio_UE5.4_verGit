#include "Component/CObservableOverlayComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

UCObservableOverlayComponent::UCObservableOverlayComponent()
{
}

void UCObservableOverlayComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	BuildObservableOverlayPolicies();
}

void UCObservableOverlayComponent::WriteObservableOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const
{
	for (const TScriptInterface<IObservableOverlayPolicy>& policy : ObservableOverlayPolicies)
	{
		const IObservableOverlayPolicy* overlayPolicy = policy.GetInterface();
		if (!overlayPolicy) continue;

		// Each overlay policy writes its runtime state into the shared snapshot.
		overlayPolicy->WriteObservableOverlaySnapshot(OutSnapshot);
	}
}

bool UCObservableOverlayComponent::NotifyObservableOverlayEvent(const FObservableOverlayEventContext& InContext)
{
	if (!InContext.IsValidMinimal()) return false;

	for (const TScriptInterface<IObservableOverlayPolicy>& policy : ObservableOverlayPolicies)
	{
		IObservableOverlayPolicy* overlayPolicy = policy.GetInterface();
		if (!overlayPolicy) continue;
		if (!overlayPolicy->CanHandleObservableOverlayEvent(InContext)) continue;

		return overlayPolicy->HandleObservableOverlayEvent(InContext);
	}

	return false;
}

bool UCObservableOverlayComponent::ApplyObservableOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings)
{
	for (const EObservableOverlayHandling handling : InHandlings)
	{
		if (!ApplyObservableOverlayHandling(handling)) return false;
	}

	return true;
}

bool UCObservableOverlayComponent::ApplyObservableOverlayHandling(EObservableOverlayHandling InHandling)
{
	if (InHandling == EObservableOverlayHandling::None) return true;

	for (const TScriptInterface<IObservableOverlayPolicy>& policy : ObservableOverlayPolicies)
	{
		IObservableOverlayPolicy* overlayPolicy = policy.GetInterface();
		if (!overlayPolicy) continue;
		if (!overlayPolicy->CanApplyObservableOverlayHandling(InHandling)) continue;

		return overlayPolicy->ApplyObservableOverlayHandling(InHandling);
	}

	return false;
}

void UCObservableOverlayComponent::BuildObservableOverlayPolicies()
{
	ObservableOverlayPolicies.Empty();

	TArray<UActorComponent*> ownerComponents;
	OwnerCharacter_Cached->GetComponents(ownerComponents);

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
}
