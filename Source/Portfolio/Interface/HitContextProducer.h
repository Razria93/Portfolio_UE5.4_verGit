#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Type/CWeaponStructure.h"
#include "HitContextProducer.generated.h"

UINTERFACE(MinimalAPI)
class UHitContextProducer : public UInterface
{
	GENERATED_BODY()
};

class PORTFOLIO_API IHitContextProducer
{
	GENERATED_BODY()

public:
	virtual FAttachmentContext GetAttachmentContext() const = 0;
	virtual FEquipmentContext GetEquipmentContext() const = 0;
	virtual FActionContext GetActionContext() const = 0;

public:
	virtual void SetAttachmentContext(FAttachmentContext InAttachmentContext) = 0;
	virtual void SetEquipmentContext(FEquipmentContext InEquipmentContext) = 0;
	virtual void SetActionContext(FActionContext InActionContext) = 0;
};
