#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Type/CActionOrchestrationTypes.h"
#include "CBTTask_RequestMovementIntent.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_RequestMovementIntent : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UCBTTask_RequestMovementIntent();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	EMovementActionIntent MovementIntent = EMovementActionIntent::Run;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
