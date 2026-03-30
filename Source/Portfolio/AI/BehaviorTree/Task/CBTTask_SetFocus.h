#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_SetFocus.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_SetFocus : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_SetFocus();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
