#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetContextProducer.generated.h"

UINTERFACE(MinimalAPI)
class UTargetContextProducer : public UInterface
{
	GENERATED_BODY()
};

class PORTFOLIO_API ITargetContextProducer
{
	GENERATED_BODY()

public:
	virtual int GetTargetPriority() const = 0;
};
