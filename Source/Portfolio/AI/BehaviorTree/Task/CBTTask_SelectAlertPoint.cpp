#include "AI/BehaviorTree/Task/CBTTask_SelectAlertPoint.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Blackboard/CAIKey.h"

UCBTTask_SelectAlertPoint::UCBTTask_SelectAlertPoint()
{
	NodeName = TEXT("Select Alert Point");
}

EBTNodeResult::Type UCBTTask_SelectAlertPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	AAIController* aiController = OwnerComp.GetAIOwner();
	if (!IsValid(aiController)) return EBTNodeResult::Failed;

	APawn* pawn = aiController->GetPawn();
	if (!IsValid(pawn)) return EBTNodeResult::Failed;

	AActor* target = Cast<AActor>(blackboardComp->GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
	if (!IsValid(target)) return EBTNodeResult::Failed;

	bool bUseAlertStep = blackboardComp->GetValueAsBool(CAIKey::Alert::bUseAlertStep.KeyName);
	float stepForwardDistance = blackboardComp->GetValueAsFloat(CAIKey::Alert::StepForwardDistance.KeyName);
	float stepSideDistance = blackboardComp->GetValueAsFloat(CAIKey::Alert::StepSideDistance.KeyName);

	FVector unitVec_ToTarget = (target->GetActorLocation() - pawn->GetActorLocation()).GetSafeNormal2D();
	FVector unitVec_Right = FVector::CrossProduct(FVector::UpVector, unitVec_ToTarget);
	float side = FMath::RandBool() ? 1.f : -1.f;

	const FVector alertLocation = pawn->GetActorLocation() + (unitVec_ToTarget * stepForwardDistance) + (unitVec_Right * side * stepSideDistance);

	blackboardComp->SetValueAsVector(CAIKey::Alert::AlertStepLocation.KeyName, alertLocation);
	return EBTNodeResult::Succeeded;
}
