#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "CBTDecorator_HasValidTarget.generated.h"

UCLASS()
class PORTFOLIO_API UCBTDecorator_HasValidTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UCBTDecorator_HasValidTarget();

protected:
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bRequireLOS = false;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
