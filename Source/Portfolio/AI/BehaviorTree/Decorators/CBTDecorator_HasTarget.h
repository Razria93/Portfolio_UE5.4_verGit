#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "CBTDecorator_HasTarget.generated.h"

UCLASS()
class PORTFOLIO_API UCBTDecorator_HasTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UCBTDecorator_HasTarget();

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard|Read")
	FBlackboardKeySelector TargetActorKey; // Cached KeyName

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
