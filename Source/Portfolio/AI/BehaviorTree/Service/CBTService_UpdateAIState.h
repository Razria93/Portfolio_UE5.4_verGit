#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CAIStateStructure.h"
#include "CBTService_UpdateAIState.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateAIState : public UBTService
{
	GENERATED_BODY()
	
public:
	UCBTService_UpdateAIState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	EAIStateType DecideNextAIStateType(UBlackboardComponent* InBlackboard, float InCurrentTime) const;
	bool ChangeAIStateType(UBlackboardComponent* InBlackboard, EAIStateType InNextAIStateType) const;
};
