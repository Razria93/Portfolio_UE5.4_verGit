#include "AI/BehaviorTree/Tasks/CBTTask_SetAIState.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKeys.h"

UCBTTask_SetAIState::UCBTTask_SetAIState()
{
	NodeName = "Set AI State";
	KeySeletor.SelectedKeyName = CAIKeys::AIStateType;
}

EBTNodeResult::Type UCBTTask_SetAIState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();
	if (!blackboard) return EBTNodeResult::Failed;

	blackboard->SetValueAsEnum(KeySeletor.SelectedKeyName, static_cast<uint8>(SetState));

	return EBTNodeResult::Succeeded;
}
