#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_WaitEndCombatAction.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_WaitEndCombatAction : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_WaitEndCombatAction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
