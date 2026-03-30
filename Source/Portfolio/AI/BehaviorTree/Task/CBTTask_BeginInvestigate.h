#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_BeginInvestigate.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_BeginInvestigate : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_BeginInvestigate();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
