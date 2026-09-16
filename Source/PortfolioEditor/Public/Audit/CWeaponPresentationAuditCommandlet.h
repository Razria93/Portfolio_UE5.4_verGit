#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"

#include "CWeaponPresentationAuditCommandlet.generated.h"

UCLASS()
class PORTFOLIOEDITOR_API UCWeaponPresentationAuditCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UCWeaponPresentationAuditCommandlet();

	virtual int32 Main(const FString& Params) override;
};
