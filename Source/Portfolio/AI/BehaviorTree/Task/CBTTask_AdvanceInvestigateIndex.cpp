#include "AI/BehaviorTree/Task/CBTTask_AdvanceInvestigateIndex.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_AdvanceInvestigateIndex::UCBTTask_AdvanceInvestigateIndex()
{
	NodeName = TEXT("Advance Investigate Index");
}

EBTNodeResult::Type UCBTTask_AdvanceInvestigateIndex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	const int32 maxIndex = blackboardComp->GetValueAsInt(CAIKey::Investigate::InvestigateMaxIndex);
	const int32 currentIndex = blackboardComp->GetValueAsInt(CAIKey::Investigate::InvestigateIndex);
	const int32 nextIndex = currentIndex + 1;

	if (nextIndex > maxIndex)
	{
		blackboardComp->SetValueAsBool(CAIKey::Investigate::bCanInvestigate, false);
		blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex, currentIndex);
		FLog::Log(TEXT("[Index Done]"));
	}
	else
	{
		blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex, nextIndex);
	}

	return EBTNodeResult::Succeeded;
}
