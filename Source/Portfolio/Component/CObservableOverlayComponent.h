#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/ObservableOverlayPolicy.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CObservableOverlayTypes.h"
#include "Type/CExecutionTypes.h"
#include "CObservableOverlayComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCObservableOverlayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCObservableOverlayComponent();

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Injected = nullptr;

	UPROPERTY(Transient)
	TArray<TScriptInterface<IObservableOverlayPolicy>> ObservableOverlayPolicies;

	UPROPERTY(Transient)
	bool bOverlayPolicyRegistryDirty = true;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

private:
	bool ValidateRequiredComponentReferences() const;

public:
	// Overlay Snapshot
	void WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot);

	// Overlay Event
	bool ApplyOverlayEvent(const FObservableOverlayEventContext& InContext);

	// Overlay Handling
	bool ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings);
	bool ApplyOverlayHandling(EObservableOverlayHandling InHandling);

private:
	// Policy Registry
	void MarkPolicyRegistryDirty();
	void RefreshPolicyRegistry();
	void RebuildPolicyRegistry();
};
