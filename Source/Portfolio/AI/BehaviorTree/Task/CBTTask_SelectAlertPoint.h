#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_SelectAlertPoint.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_SelectAlertPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_SelectAlertPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
