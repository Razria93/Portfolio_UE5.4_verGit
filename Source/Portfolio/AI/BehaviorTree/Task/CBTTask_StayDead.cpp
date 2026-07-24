#include "AI/BehaviorTree/Task/CBTTask_StayDead.h"

#include "ProjectGlobal.h"

UCBTTask_StayDead::UCBTTask_StayDead()
{
	NodeName = TEXT("Stay Dead");
}

EBTNodeResult::Type UCBTTask_StayDead::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}
