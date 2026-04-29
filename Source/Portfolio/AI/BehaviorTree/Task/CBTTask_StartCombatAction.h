#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CBTTask_StartCombatAction.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StartCombatAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_StartCombatAction();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	ECombatActionIntent CombatActionIntent = ECombatActionIntent::None;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bStopMovementOnStart = true;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
