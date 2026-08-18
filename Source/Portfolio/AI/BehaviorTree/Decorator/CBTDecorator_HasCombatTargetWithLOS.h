#pragma once

#include "CoreMinimal.h"
#include "AI/BehaviorTree/Decorator/CBTDecorator_HasCombatTarget.h"
#include "CBTDecorator_HasCombatTargetWithLOS.generated.h"

UCLASS()
class PORTFOLIO_API UCBTDecorator_HasCombatTargetWithLOS : public UCBTDecorator_HasCombatTarget
{
	GENERATED_BODY()

public:
	UCBTDecorator_HasCombatTargetWithLOS();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
