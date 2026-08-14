#include "AI/BehaviorTree/Task/CBTTask_SetFocus.h"

#include "ProjectGlobal.h"

#include "Character/Enemy/CEnemy.h"
#include "Component/CCombatTargetComponent.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_SetFocus::UCBTTask_SetFocus()
{
	NodeName = TEXT("Set Focus");
}

EBTNodeResult::Type UCBTTask_SetFocus::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	const ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	const UCCombatTargetComponent* combatTargetComp = IsValid(enemy) ? enemy->GetCombatTargetComp() : nullptr;
	AActor* target = IsValid(combatTargetComp) ? combatTargetComp->GetCombatTargetSnapshot().TargetActor : nullptr;
	if (!IsValid(target)) return EBTNodeResult::Failed;

	aiController->SetFocus(target, EAIFocusPriority::Gameplay);
	return EBTNodeResult::Succeeded;
}
