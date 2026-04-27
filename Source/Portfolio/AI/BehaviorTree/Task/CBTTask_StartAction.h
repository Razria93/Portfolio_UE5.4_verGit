#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CBTTask_StartAction.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_StartAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_StartAction();

private:
	UPROPERTY(EditAnywhere, Category = "Config")
	ECombatActionIntent CombatActionIntent = ECombatActionIntent::ComboAttack;

	UPROPERTY(EditAnywhere, Category = "Config")
	bool bStopMovementOnStart = true;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
