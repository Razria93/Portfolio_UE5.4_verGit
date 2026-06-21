#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Type/CWeaponStructure.h"
#include "ObservableOverlayPolicy.generated.h"

UINTERFACE(MinimalAPI)
class UObservableOverlayPolicy : public UInterface
{
	GENERATED_BODY()
};

class PORTFOLIO_API IObservableOverlayPolicy
{
	GENERATED_BODY()

public:
	virtual void WriteOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const = 0;
	virtual bool CanApplyOverlayEvent(const FObservableOverlayEventContext& InContext) const = 0;
	virtual bool ApplyOverlayEvent(const FObservableOverlayEventContext& InContext) = 0;
	virtual bool CanApplyOverlayHandling(EObservableOverlayHandling InHandling) const = 0;
	virtual bool ApplyOverlayHandling(EObservableOverlayHandling InHandling) = 0;
};
