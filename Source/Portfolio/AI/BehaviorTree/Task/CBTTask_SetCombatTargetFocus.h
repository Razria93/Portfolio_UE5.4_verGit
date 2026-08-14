#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_SetCombatTargetFocus.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_SetCombatTargetFocus : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_SetCombatTargetFocus();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
