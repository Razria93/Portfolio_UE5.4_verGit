#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_StayDead.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StayDead : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_StayDead();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
