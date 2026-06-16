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
	virtual void WriteObservableOverlaySnapshot(FObservableOverlaySnapshot& OutSnapshot) const = 0;
	virtual bool CanApplyObservableOverlayHandling(EObservableOverlayHandling InHandling) const = 0;
	virtual bool ApplyObservableOverlayHandling(EObservableOverlayHandling InHandling) = 0;
};
