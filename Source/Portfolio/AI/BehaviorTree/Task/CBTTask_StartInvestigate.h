#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_StartInvestigate.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StartInvestigate : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_StartInvestigate();

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
