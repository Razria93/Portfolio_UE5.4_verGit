#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_AdvanceInvestigateIndex.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_AdvanceInvestigateIndex : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_AdvanceInvestigateIndex();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
