#include "AI/BehaviorTree/Task/CBTTask_ClearFocus.h"

#include "ProjectGlobal.h"

#include "AIController.h"
#include "Character/Enemy/CEnemy.h"
#include "Component/CEnemyCombatTargetFacingComponent.h"

UCBTTask_ClearFocus::UCBTTask_ClearFocus()
{
	NodeName = TEXT("Clear Focus");
}

EBTNodeResult::Type UCBTTask_ClearFocus::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	ACEnemy* enemy = Cast<ACEnemy>(aiController->GetPawn());
	UCEnemyCombatTargetFacingComponent* facingComp = IsValid(enemy) ? enemy->GetEnemyCombatTargetFacingComp() : nullptr;
	if (IsValid(facingComp))
	{
		facingComp->ClearGameplayFocusFromExternal(aiController, TEXT("BT.ClearFocus"));
	}
	else
	{
		aiController->ClearFocus(EAIFocusPriority::Gameplay);
	}
	return EBTNodeResult::Succeeded;
}
