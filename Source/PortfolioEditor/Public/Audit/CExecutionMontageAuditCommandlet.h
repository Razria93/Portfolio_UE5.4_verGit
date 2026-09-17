#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"

#include "CExecutionMontageAuditCommandlet.generated.h"

UCLASS()
class PORTFOLIOEDITOR_API UCExecutionMontageAuditCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UCExecutionMontageAuditCommandlet();

	virtual int32 Main(const FString& Params) override;
};
