#include "AI/BehaviorTree/Task/CBTTask_RequestCombatTargetSelection.h"

#include "AI/Blackboard/CAIKey.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CEnemyTargetSelectionComponent.h"
#include "Type/CCombatTargetTypes.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_RequestCombatTargetSelection::UCBTTask_RequestCombatTargetSelection()
{
	NodeName = TEXT("Request Combat Target Selection");
}

EBTNodeResult::Type UCBTTask_RequestCombatTargetSelection::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	const AAIController* aiController = OwnerComp.GetAIOwner();
	ACEnemy* enemy = IsValid(aiController) ? Cast<ACEnemy>(aiController->GetPawn()) : nullptr;
	UCEnemyTargetSelectionComponent* selectionComp = IsValid(enemy) ? enemy->GetEnemyTargetSelectionComp() : nullptr;

	if (!IsValid(blackboardComp) || !IsValid(selectionComp)) return EBTNodeResult::Failed;

	AActor* candidate = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Perception::PerceivedTargetActor.KeyName));
	const FEnemyTargetSelectionResult result = selectionComp->RequestSelectCombatTarget(candidate, ECombatTargetChangeReason::AIDecision);

	return result.Decision == EEnemyTargetSelectionDecision::Committed || result.Decision == EEnemyTargetSelectionDecision::Unchanged ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
