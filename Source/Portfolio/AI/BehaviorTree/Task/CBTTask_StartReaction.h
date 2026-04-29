#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_StartReaction.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StartReaction : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_StartReaction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
