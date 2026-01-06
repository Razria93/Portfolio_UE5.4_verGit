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
	virtual const FOverlapContext& GetLastOverlapContext() const = 0;
	virtual const FAttachmentContext& GetLastAttachmentContext() const = 0;
	virtual const FEquipmentContext& GetLastEquipmentContext() const = 0;
	virtual const FActionContext& GetLastActionContext() const = 0;

public:
	virtual void SetLastOverlapContext(const FOverlapContext& InOverlapContext) = 0;
	virtual void SetLastAttachmentContext(const FAttachmentContext& InAttachmentContext) = 0;
	virtual void SetLastEquipmentContext(const FEquipmentContext& InEquipmentContext) = 0;
	virtual void SetLastActionContext(const FActionContext& InActionContext) = 0;
};
