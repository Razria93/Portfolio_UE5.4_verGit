#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetContextProvider.generated.h"

UINTERFACE(MinimalAPI)
class UTargetContextProvider : public UInterface
{
	GENERATED_BODY()
};

class PORTFOLIO_API ITargetContextProvider
{
	GENERATED_BODY()

public:
	virtual int GetTargetPriority() const = 0;
};
