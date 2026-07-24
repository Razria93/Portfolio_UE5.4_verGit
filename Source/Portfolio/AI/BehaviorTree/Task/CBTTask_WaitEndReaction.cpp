#include "AI/BehaviorTree/Task/CBTTask_WaitEndReaction.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"

#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_WaitEndReaction::UCBTTask_WaitEndReaction()
{
	NodeName = TEXT("Wait End Reaction");
	bNotifyTick = true;
}

EBTNodeResult::Type UCBTTask_WaitEndReaction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UCBTTask_WaitEndReaction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const bool bIsActiveReaction = blackboardComp->GetValueAsBool(CAIKey::Reaction::bIsActiveReaction.KeyName);

	if (!bIsActiveReaction)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
