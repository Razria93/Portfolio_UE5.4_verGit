#include "AI/BehaviorTree/Task/CBTTask_BeginInvestigate.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_BeginInvestigate::UCBTTask_BeginInvestigate()
{
	NodeName = TEXT("Begin Investigate");
}

EBTNodeResult::Type UCBTTask_BeginInvestigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	UWorld* world = OwnerComp.GetWorld();
	if (!IsValid(world)) return EBTNodeResult::Failed;

	const FVector lastKnownLocation = blackboardComp->GetValueAsVector(CAIKey::Perception::LastKnownLocation);

	blackboardComp->SetValueAsBool(CAIKey::Investigate::bCanInvestigate, true);
	blackboardComp->SetValueAsBool(CAIKey::Investigate::bIsInvestigating, true);

	blackboardComp->SetValueAsVector(CAIKey::Investigate::InvestigateLocation, lastKnownLocation);
	blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex, 0);

	return EBTNodeResult::Succeeded;
}
