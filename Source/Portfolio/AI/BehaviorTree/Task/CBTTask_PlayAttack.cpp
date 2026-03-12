#include "AI/BehaviorTree/Task/CBTTask_PlayAttack.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_PlayAttack::UCBTTask_PlayAttack()
{
	NodeName = TEXT("Play Attack");
}

EBTNodeResult::Type UCBTTask_PlayAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FLog::Log(TEXT("=== Execute Play Attack ==="));

	return EBTNodeResult::Succeeded;
}
