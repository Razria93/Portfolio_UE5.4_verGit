#include "AI/BehaviorTree/Task/CBTTask_StartRevive.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "Character/Enemy/CEnemy.h"

UCBTTask_StartRevive::UCBTTask_StartRevive()
{
	NodeName = TEXT("Start Revive");
}

EBTNodeResult::Type UCBTTask_StartRevive::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	return enemy->TryStartRevive(ReviveHP) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}