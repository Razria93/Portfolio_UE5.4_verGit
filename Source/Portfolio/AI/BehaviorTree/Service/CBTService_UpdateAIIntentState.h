#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CStateStructure.h"
#include "CBTService_UpdateAIIntentState.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateAIIntentState : public UBTService
{
	GENERATED_BODY()
	
public:
	UCBTService_UpdateAIIntentState();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	EAIIntentState DecideNextAIIntentState(UBlackboardComponent* InBlackboardComp, float InCurrentTime);
	bool ChangeAIIntentState(UBlackboardComponent* InBlackboardComp, EAIIntentState InNextAIIntentState);
	void UpdateAIIntentStateTransition(UBlackboardComponent* InBlackboardComp, EAIIntentState InCurrentAIIntentState, EAIIntentState InNextAIIntentState);
};
