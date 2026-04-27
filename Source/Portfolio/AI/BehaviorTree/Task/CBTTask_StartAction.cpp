#include "AI/BehaviorTree/Task/CBTTask_StartAction.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/CEnemy.h"

#include "AI/BlackBoard/CAIKey.h"

#include "Type/CActionOrchestrationStructure.h"

UCBTTask_StartAction::UCBTTask_StartAction()
{
	NodeName = TEXT("Start Action");
}

EBTNodeResult::Type UCBTTask_StartAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	const bool bCanAttack = blackboardComp->GetValueAsBool(CAIKey::Engage::bCanAttack);
	if (!bCanAttack) return EBTNodeResult::Failed;

	if (bStopMovementOnStart)
	{
		aiController->StopMovement();
	}

	const FActionRequestResult requestResult = enemy->HandleAICombatAction(CombatActionIntent);
	if (!requestResult.IsAccepted()) return EBTNodeResult::Failed;

	blackboardComp->SetValueAsBool(CAIKey::Engage::bCanAttack, false);

	return EBTNodeResult::Succeeded;
}
