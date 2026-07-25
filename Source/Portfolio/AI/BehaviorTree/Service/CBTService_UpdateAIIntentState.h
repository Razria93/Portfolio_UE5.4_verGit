#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "Type/CStateTypes.h"
#include "CBTService_UpdateAIIntentState.generated.h"

UCLASS()
class PORTFOLIO_API UCBTService_UpdateAIIntentState : public UBTService
{
	GENERATED_BODY()
	
public:
	UCBTService_UpdateAIIntentState();

protected:
	// Lifecycle
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void ScheduleNextTick(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// Intent Decision
	EAIIntentState DecideNextAIIntentState(UBlackboardComponent* InBlackboardComp, float InCurrentTime);
	bool ChangeAIIntentState(UBlackboardComponent* InBlackboardComp, EAIIntentState InNextAIIntentState);

	// Intent Transition
	void UpdateAIIntentStateTransition(UBlackboardComponent* InBlackboardComp, EAIIntentState InCurrentAIIntentState, EAIIntentState InNextAIIntentState);
};
