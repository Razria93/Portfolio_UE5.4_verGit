#include "AI/BehaviorTree/Task/CBTTask_WaitDeadState.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CHealthComponent.h"

UCBTTask_WaitDeadState::UCBTTask_WaitDeadState()
{
	NodeName = TEXT("Wait Dead State");
	bNotifyTick = true;
}

EBTNodeResult::Type UCBTTask_WaitDeadState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UCBTTask_WaitDeadState::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	UCHealthComponent* healthComp = enemy->GetHealthComp();
	if (!IsValid(healthComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (healthComp->GetDeadState() == TargetDeadState)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}