#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/ObservableOverlayPolicy.h"
#include "Type/CWeaponStructure.h"
#include "CObservableOverlayComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCObservableOverlayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCObservableOverlayComponent();

private:
	UPROPERTY(Transient)
	class ACharacter* OwnerCharacter_Cached = nullptr;

	UPROPERTY(Transient)
	TArray<TScriptInterface<IObservableOverlayPolicy>> ObservableOverlayPolicies;

protected:
	void BeginPlay() override;

public:
	void WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const;
	bool ApplyOverlayEvent(const FObservableOverlayEventContext& InContext);
	bool ApplyOverlayHandlings(const TArray<EObservableOverlayHandling>& InHandlings);
	bool ApplyOverlayHandling(EObservableOverlayHandling InHandling);

private:
	void BuildObservableOverlayPolicies();
};
