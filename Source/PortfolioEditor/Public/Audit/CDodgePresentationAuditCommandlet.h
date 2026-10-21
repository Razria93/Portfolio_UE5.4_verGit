#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CDodgePresentationAuditCommandlet.generated.h"

/** Read-only inspection of the restored Fresnel and modular presentation assets. */
UCLASS()
class PORTFOLIOEDITOR_API UCDodgePresentationAuditCommandlet : public UCommandlet
{
	GENERATED_BODY()
public:
	UCDodgePresentationAuditCommandlet();
	int32 Main(const FString& Params) override;
};
