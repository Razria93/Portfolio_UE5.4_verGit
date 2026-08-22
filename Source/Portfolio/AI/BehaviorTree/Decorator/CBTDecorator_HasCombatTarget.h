#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "CBTDecorator_HasCombatTarget.generated.h"

UCLASS()
class PORTFOLIO_API UCBTDecorator_HasCombatTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UCBTDecorator_HasCombatTarget();

protected:
	bool HasCombatTarget(const UBehaviorTreeComponent& InOwnerComp) const;
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
