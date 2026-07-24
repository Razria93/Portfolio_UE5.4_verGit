#include "AI/BehaviorTree/Task/CBTTask_StartInvestigate.h"

#include "ProjectGlobal.h"

#include "AI/Blackboard/CAIKey.h"

#include "BehaviorTree/BlackboardComponent.h"

UCBTTask_StartInvestigate::UCBTTask_StartInvestigate()
{
	NodeName = TEXT("Start Investigate");
}

EBTNodeResult::Type UCBTTask_StartInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	UWorld* world = OwnerComp.GetWorld();
	if (!IsValid(world)) return EBTNodeResult::Failed;

	const FVector lastKnownLocation = blackboardComp->GetValueAsVector(CAIKey::Perception::LastKnownLocation.KeyName);

	blackboardComp->SetValueAsBool(CAIKey::Investigate::bShouldInvestigate.KeyName, false);
	blackboardComp->SetValueAsBool(CAIKey::Investigate::bIsInvestigating.KeyName, true);
	blackboardComp->SetValueAsBool(CAIKey::Investigate::bShouldEndInvestigate.KeyName, false);

	blackboardComp->SetValueAsVector(CAIKey::Investigate::InvestigateLocation.KeyName, lastKnownLocation);
	blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex.KeyName, 0);

	return EBTNodeResult::Succeeded;
}
