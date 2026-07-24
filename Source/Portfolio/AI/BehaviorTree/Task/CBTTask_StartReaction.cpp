#include "AI/BehaviorTree/Task/CBTTask_StartReaction.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"

#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_StartReaction::UCBTTask_StartReaction()
{
	NodeName = TEXT("Start Reaction");
}

EBTNodeResult::Type UCBTTask_StartReaction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	const bool bIsActiveReaction = blackboardComp->GetValueAsBool(CAIKey::Reaction::bIsActiveReaction.KeyName);

	return bIsActiveReaction ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
