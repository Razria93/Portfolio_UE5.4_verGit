#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_EndAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_EndAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_EndAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
