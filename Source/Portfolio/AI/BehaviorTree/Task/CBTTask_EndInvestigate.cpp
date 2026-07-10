#include "AI/BehaviorTree/Task/CBTTask_EndInvestigate.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Blackboard/CAIKey.h"

UCBTTask_EndInvestigate::UCBTTask_EndInvestigate()
{
	NodeName = TEXT("End Investigate");
}

EBTNodeResult::Type UCBTTask_EndInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8*)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	blackboardComp->SetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName, false);
	blackboardComp->SetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName, false);

	blackboardComp->ClearValue(CAIKey::Investigate::InvestigateLocation.KeyName);
	blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex.KeyName, -1);

	blackboardComp->ClearValue(CAIKey::Perception::LastSeenTime.KeyName);
	blackboardComp->ClearValue(CAIKey::Perception::LastKnownLocation.KeyName);

	return EBTNodeResult::Succeeded;
}