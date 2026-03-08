#include "AI/BehaviorTree/Task/CBTTask_EndInvestigate.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_EndInvestigate::UCBTTask_EndInvestigate()
{
	NodeName = TEXT("End Investigate");
}

EBTNodeResult::Type UCBTTask_EndInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8*)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	blackboardComp->SetValueAsBool(CAIKey::Investigate::bCanInvestigate, false);
	blackboardComp->SetValueAsBool(CAIKey::Investigate::bIsInvestigating, false);

	blackboardComp->ClearValue(CAIKey::Investigate::InvestigateLocation);
	blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex, -1);

	blackboardComp->ClearValue(CAIKey::Perception::LastSeenTime);
	blackboardComp->ClearValue(CAIKey::Perception::LastKnownLocation);

	return EBTNodeResult::Succeeded;
}