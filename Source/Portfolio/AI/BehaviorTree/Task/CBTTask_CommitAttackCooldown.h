#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_CommitAttackCooldown.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_CommitAttackCooldown : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_CommitAttackCooldown();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
