#include "AI/BehaviorTree/Task/CBTTask_WaitEndCombatAction.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"

#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_WaitEndCombatAction::UCBTTask_WaitEndCombatAction()
{
	NodeName = TEXT("Wait End Combat Action");
	bNotifyTick = true;
}

EBTNodeResult::Type UCBTTask_WaitEndCombatAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		return EBTNodeResult::Failed;
	}

	if (!blackboardComp->GetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName))
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UCBTTask_WaitEndCombatAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (!blackboardComp->GetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}
