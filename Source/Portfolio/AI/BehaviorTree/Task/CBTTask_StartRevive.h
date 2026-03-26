#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "CBTTask_StartRevive.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StartRevive : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_StartRevive();

public:
	UPROPERTY(EditAnywhere)
	float ReviveHP = 30.f;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
