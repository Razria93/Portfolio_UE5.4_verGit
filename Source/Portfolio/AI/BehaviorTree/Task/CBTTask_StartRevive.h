#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_StartRevive.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StartRevive : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_StartRevive();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
