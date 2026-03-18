#include "AI/BehaviorTree/Task/CBTTask_WaitEndReaction.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CReactionComponent.h"

UCBTTask_WaitEndReaction::UCBTTask_WaitEndReaction()
{
	NodeName = TEXT("Wait Reaction End");
	bNotifyTick = true;
}

EBTNodeResult::Type UCBTTask_WaitEndReaction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	UCReactionComponent* reactionComp = enemy->GetReactionComponent();
	if (!IsValid(reactionComp)) return EBTNodeResult::Failed;

	// Succeseded Task
	if (!reactionComp->HasActiveReactionContext())
		return EBTNodeResult::Succeeded;

	// Executing Task
	return EBTNodeResult::InProgress;
}

void UCBTTask_WaitEndReaction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
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

	UCReactionComponent* reactionComp = enemy->GetReactionComponent();
	if (!IsValid(reactionComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!reactionComp->HasActiveReactionContext())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
