#include "AI/BehaviorTree/Task/CBTTask_WaitEndReaction.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CReactionComponent.h"

#include "AI/BlackBoard/CAIKey.h"

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

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UCReactionComponent* reactionComp = enemy->GetReactionComp();
	if (!IsValid(reactionComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!reactionComp->IsActiveReaction())
	{
		FLog::Log(TEXT("[WaitEndReaction|TickTask] Current Reaction Ended"));

		// Current reaction ended
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
