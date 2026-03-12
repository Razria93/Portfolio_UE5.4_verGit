#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_PlayAttack.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_PlayAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_PlayAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
