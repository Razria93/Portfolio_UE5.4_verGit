#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_SelectAttackIndex.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_SelectAttackIndex : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_SelectAttackIndex();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	int32 AttackCount = 1;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bLoop = true;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
