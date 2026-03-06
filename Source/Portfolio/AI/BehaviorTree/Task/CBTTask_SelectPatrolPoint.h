#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_SelectPatrolPoint.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_SelectPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

	/********************************************
	*
	* [NOTE] Regacy Task
	* - replaced UCBTService_UpdatePatrolContext
	* 
	********************************************/

public:
	UCBTTask_SelectPatrolPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
