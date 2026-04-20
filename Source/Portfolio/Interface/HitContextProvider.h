#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Type/CWeaponStructure.h"
#include "HitContextProvider.generated.h"

UINTERFACE(MinimalAPI)
class UHitContextProvider : public UInterface
{
	GENERATED_BODY()
};

class PORTFOLIO_API IHitContextProvider
{
	GENERATED_BODY()

public:
	virtual const FOverlapContext& GetLastOverlapContext() const = 0;
	virtual const FWeaponActorContext& GetLastWeaponActorContext() const = 0;
	virtual const FEquipmentContext& GetLastEquipmentContext() const = 0;
	virtual const FActionContext& GetLastActionContext() const = 0;

public:
	virtual void SetLastOverlapContext(const FOverlapContext& InOverlapContext) = 0;
	virtual void SetLastWeaponActorContext(const FWeaponActorContext& InWeaponActorContext) = 0;
	virtual void SetLastEquipmentContext(const FEquipmentContext& InEquipmentContext) = 0;
	virtual void SetLastActionContext(const FActionContext& InActionContext) = 0;
};
