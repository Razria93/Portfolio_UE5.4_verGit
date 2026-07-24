#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Type/CHealthTypes.h"
#include "CBTTask_WaitDeadState.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_WaitDeadState : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_WaitDeadState();

protected:
	UPROPERTY(EditAnywhere, Category = "Dead")
	EDeadState TargetDeadState = EDeadState::Dead;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
