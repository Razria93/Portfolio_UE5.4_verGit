#include "AI/BehaviorTree/Task/CBTTask_StartCombatAction.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Character/Enemy/CEnemy.h"

#include "AI/BlackBoard/CAIKey.h"

#include "Type/CActionOrchestrationStructure.h"

UCBTTask_StartCombatAction::UCBTTask_StartCombatAction()
{
	NodeName = TEXT("Start Combat Action");
}

EBTNodeResult::Type UCBTTask_StartCombatAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	if (!IsValid(enemy)) return EBTNodeResult::Failed;

	const bool bCanCombatAction = blackboardComp->GetValueAsBool(CAIKey::Engage::bCanCombatAction);
	if (!bCanCombatAction) return EBTNodeResult::Failed;

	const bool bIsCombatAction = blackboardComp->GetValueAsBool(CAIKey::Engage::bIsCombatAction);
	if (bIsCombatAction) return EBTNodeResult::Failed;

	if (bStopMovementOnStart)
	{
		aiController->StopMovement();
	}

	const FActionRequestResult requestResult = enemy->HandleAICombatAction(CombatActionIntent);
	if (!requestResult.IsStartedResult()) return EBTNodeResult::Failed;

	const float currentTime = OwnerComp.GetWorld()->GetTimeSeconds();
	const float nextCombatActionTime = currentTime + enemy->GetCombatActionCooldown();
	
	// Set Cooldown
	blackboardComp->SetValueAsFloat(CAIKey::Engage::NextCombatActionTime, nextCombatActionTime);

	return EBTNodeResult::Succeeded;
}
