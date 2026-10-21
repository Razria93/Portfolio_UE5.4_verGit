#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CDodgeAssetsCommandlet.generated.h"

UCLASS()
class PORTFOLIOEDITOR_API UCDodgeAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()
public:
	UCDodgeAssetsCommandlet();
	int32 Main(const FString& Params) override;
};
