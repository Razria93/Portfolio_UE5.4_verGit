#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_InitCombat.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_InitCombat : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_InitCombat();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
