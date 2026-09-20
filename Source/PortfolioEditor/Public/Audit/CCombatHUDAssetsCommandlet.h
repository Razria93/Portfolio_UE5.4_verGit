#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CCombatHUDAssetsCommandlet.generated.h"

// Explicit, create-only import of the HUD's documented source resources.
UCLASS()
class PORTFOLIOEDITOR_API UCCombatHUDAssetsCommandlet : public UCommandlet
{
	GENERATED_BODY()
public:
	UCCombatHUDAssetsCommandlet();
	virtual int32 Main(const FString& Params) override;
};
