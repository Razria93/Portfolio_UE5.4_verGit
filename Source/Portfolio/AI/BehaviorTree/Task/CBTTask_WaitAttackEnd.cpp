#include "AI/BehaviorTree/Task/CBTTask_WaitAttackEnd.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_WaitAttackEnd::UCBTTask_WaitAttackEnd()
{
	NodeName = TEXT("Wait Attack End");
	bNotifyTick = true;
}

EBTNodeResult::Type UCBTTask_WaitAttackEnd::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		return EBTNodeResult::Failed;
	}

	if (!blackboardComp->GetValueAsBool(CAIKey::Engage::bIsAttacking))
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UCBTTask_WaitAttackEnd::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!blackboardComp->GetValueAsBool(CAIKey::Engage::bIsAttacking))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}