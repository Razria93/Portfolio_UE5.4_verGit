#include "AI/BehaviorTree/Task/CBTTask_StartInvestigate.h"
#include "ProjectGlobal.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

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

	const FVector lastKnownLocation = blackboardComp->GetValueAsVector(CAIKey::Perception::LastKnownLocation);

	blackboardComp->SetValueAsBool(CAIKey::Investigate::bIsInvestigating, true);
	blackboardComp->SetValueAsVector(CAIKey::Investigate::InvestigateLocation, lastKnownLocation);
	blackboardComp->SetValueAsInt(CAIKey::Investigate::InvestigateIndex, 0);

	return EBTNodeResult::Succeeded;
}
