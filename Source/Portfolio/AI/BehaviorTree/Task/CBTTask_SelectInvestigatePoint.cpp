#include "AI/BehaviorTree/Task/CBTTask_SelectInvestigatePoint.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/BlackBoard/CAIKey.h"

UCBTTask_SelectInvestigatePoint::UCBTTask_SelectInvestigatePoint()
{
	NodeName = TEXT("Select Investigate Point");
}

EBTNodeResult::Type UCBTTask_SelectInvestigatePoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	APawn* ownerPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!IsValid(ownerPawn)) return EBTNodeResult::Failed;

	const FVector radius = blackboardComp->GetValueAsVector(CAIKey::Perception::LastKnownLocation);
	const int32 maxIndex = blackboardComp->GetValueAsInt(CAIKey::Investigate::InvestigateMaxIndex);
	const int32 index = blackboardComp->GetValueAsInt(CAIKey::Investigate::InvestigateIndex);
	
	if (index < 0 || index > maxIndex) return EBTNodeResult::Failed;

	const FVector lastKnownLocation = blackboardComp->GetValueAsVector(CAIKey::Perception::LastKnownLocation);
	const FVector rightVector = ownerPawn->GetActorRightVector();
	
	FVector investigateLocation = lastKnownLocation;

	// Step 0: center, Step 1: right, Step 2: left
	if (index == 0) investigateLocation = lastKnownLocation;
	else if (index == 1) investigateLocation = lastKnownLocation + rightVector * radius;
	else if (index == 2) investigateLocation = lastKnownLocation - rightVector * radius;

	blackboardComp->SetValueAsVector(CAIKey::Investigate::InvestigateLocation, investigateLocation);

	return EBTNodeResult::Succeeded;
}
