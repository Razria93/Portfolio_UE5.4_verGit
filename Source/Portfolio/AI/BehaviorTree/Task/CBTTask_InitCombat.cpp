#include "AI/BehaviorTree/Task/CBTTask_InitCombat.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_InitCombat::UCBTTask_InitCombat()
{
	NodeName = TEXT("Init Combat");
}

EBTNodeResult::Type UCBTTask_InitCombat::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackBoardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackBoardComp)) return EBTNodeResult::Failed;

	blackBoardComp->SetValueAsBool(CAIKey::Combat::bCombatInitialized, true);
	blackBoardComp->SetValueAsInt(CAIKey::Combat::AttackIndex, -1);

	return EBTNodeResult::Succeeded;
}
