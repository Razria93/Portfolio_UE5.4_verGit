#include "AI/BehaviorTree/Tasks/CBTTask_SetAIState.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_SetAIState::UCBTTask_SetAIState()
{
	NodeName = "Set AI State";
	AIStateTypeKey.SelectedKeyName = CAIKey::StateType::AIStateType;
}

EBTNodeResult::Type UCBTTask_SetAIState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboard = OwnerComp.GetBlackboardComponent();
	if (!blackboard) return EBTNodeResult::Failed;

	blackboard->SetValueAsEnum(AIStateTypeKey.SelectedKeyName, static_cast<uint8>(SetState));

	return EBTNodeResult::Succeeded;
}
