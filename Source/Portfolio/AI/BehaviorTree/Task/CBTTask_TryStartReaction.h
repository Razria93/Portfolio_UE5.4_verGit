#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_TryStartReaction.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_TryStartReaction : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_TryStartReaction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
