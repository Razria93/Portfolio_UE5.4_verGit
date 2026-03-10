#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_ClearFocus.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_ClearFocus : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_ClearFocus();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
