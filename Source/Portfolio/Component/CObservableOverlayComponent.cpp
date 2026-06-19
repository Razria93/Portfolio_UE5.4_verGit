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
		FLog::Log(FString::Printf(
			TEXT("[OverlayHandling] Try Handling=%s"),
			*UEnum::GetValueAsString(handling)));

		if (!ApplyObservableOverlayHandling(handling))
		{
			FLog::Log(FString::Printf(
				TEXT("[OverlayHandling] Failed Handling=%s"),
				*UEnum::GetValueAsString(handling)));
			return false;
		}
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
		const UObject* policyObject = policy.GetObject();
		const FString policyName = IsValid(policyObject) ? policyObject->GetName() : TEXT("InvalidPolicy");
		const bool bCanApply = overlayPolicy->CanApplyObservableOverlayHandling(InHandling);

		FLog::Log(FString::Printf(
			TEXT("[OverlayHandling] Policy=%s | Handling=%s | CanApply=%s"),
			*policyName,
			*UEnum::GetValueAsString(InHandling),
			bCanApply ? TEXT("true") : TEXT("false")));

		if (!bCanApply) continue;

		const bool bApplied = overlayPolicy->ApplyObservableOverlayHandling(InHandling);
		FLog::Log(FString::Printf(
			TEXT("[OverlayHandling] Policy=%s | Handling=%s | Applied=%s"),
			*policyName,
			*UEnum::GetValueAsString(InHandling),
			bApplied ? TEXT("true") : TEXT("false")));
		return bApplied;
	}

	FLog::Log(FString::Printf(
		TEXT("[OverlayHandling] No policy accepted Handling=%s"),
		*UEnum::GetValueAsString(InHandling)));
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
