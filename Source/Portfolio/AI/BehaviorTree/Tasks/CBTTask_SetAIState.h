#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Type/CAIStateStructure.h"
#include "CBTTask_SetAIState.generated.h"

UCLASS()
class PORTFOLIO_API UCBTTask_SetAIState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UCBTTask_SetAIState();

protected:
	UPROPERTY(EditAnywhere, Category = "Set AIState")
	FBlackboardKeySelector KeySeletor;

	UPROPERTY(EditAnywhere, Category = "Set AIState")
	EAIStateType SetState = EAIStateType::Wait;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
