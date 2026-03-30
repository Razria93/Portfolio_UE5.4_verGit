#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_EndInvestigate.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_EndInvestigate : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_EndInvestigate();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
