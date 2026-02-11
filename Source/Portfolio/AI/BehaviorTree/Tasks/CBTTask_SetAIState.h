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
	UPROPERTY(EditAnywhere, Category = "Blackboard|Set")
	EAIStateType SetState = EAIStateType::Wait;

	UPROPERTY(EditAnywhere, Category = "Blackboard|Write")
	FBlackboardKeySelector AIStateTypeKey; // Cached KeyName

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
