#include "AI/BehaviorTree/Task/CBTTask_ClearFocus.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_ClearFocus::UCBTTask_ClearFocus()
{
	NodeName = TEXT("Clear Focus");
}

EBTNodeResult::Type UCBTTask_ClearFocus::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	aiController->ClearFocus(EAIFocusPriority::Gameplay);
	return EBTNodeResult::Succeeded;
}
