#include "AI/BehaviorTree/Task/CBTTask_SelectAttackIndex.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_SelectAttackIndex::UCBTTask_SelectAttackIndex()
{
	NodeName = TEXT("Select Attack Index");
}

EBTNodeResult::Type UCBTTask_SelectAttackIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;
	if (AttackCount <= 0) return EBTNodeResult::Failed;

	const int32 currentIndex = blackboardComp->GetValueAsInt(CAIKey::Combat::AttackIndex);

	int32 nextIndex = INDEX_NONE;

	if (currentIndex < 0)
	{
		nextIndex = 0;
	}
	else if (bLoop)
	{
		nextIndex = (currentIndex + 1) % AttackCount;
	}
	else
	{
		nextIndex = FMath::Min(currentIndex + 1, AttackCount - 1);
	}

	blackboardComp->SetValueAsInt(CAIKey::Combat::AttackIndex, nextIndex);

	return EBTNodeResult::Succeeded;
}