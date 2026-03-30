#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_SetMaxWalkSpeed.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_SetMaxWalkSpeed : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_SetMaxWalkSpeed();

protected:
	UPROPERTY(EditAnywhere, Category = "Config")
	float SetMaxWalkSpeed;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
