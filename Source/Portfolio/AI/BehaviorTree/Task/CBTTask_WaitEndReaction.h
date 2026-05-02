#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_WaitEndReaction.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_WaitEndReaction : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_WaitEndReaction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
