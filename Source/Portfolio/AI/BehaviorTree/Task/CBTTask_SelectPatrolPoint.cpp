#include "AI/BehaviorTree/Task/CBTTask_SelectPatrolPoint.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Patrol/CPatrolPath.h"

#include "Type/CAIStructure.h"
#include "AI/BlackBoard/CAIKey.h"

UCBTTask_SelectPatrolPoint::UCBTTask_SelectPatrolPoint()
{
	NodeName = TEXT("Select Patrol Point");
}

EBTNodeResult::Type UCBTTask_SelectPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	APawn* ownerPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!IsValid(ownerPawn)) return EBTNodeResult::Failed;

	bool bUsePatrol = blackboardComp->GetValueAsBool(CAIKey::Patrol::bUsePatrol);
	ACPatrolPath* patrolPath = Cast<ACPatrolPath>(blackboardComp->GetValueAsObject(CAIKey::Patrol::PatrolPath));
	EPatrolMode patrolMode = static_cast<EPatrolMode>(blackboardComp->GetValueAsEnum(CAIKey::Patrol::PatrolMode));

	if (!bUsePatrol || !IsValid(patrolPath) || patrolPath->Num() <= 0 || patrolMode == EPatrolMode::None) return EBTNodeResult::Failed;
	
	int32 currentIndex = blackboardComp->GetValueAsInt(CAIKey::Patrol::PatrolIndex);
	bool bPatrolReverse = blackboardComp->GetValueAsBool(CAIKey::Patrol::bPatrolReverse);
	
	const int32 count = patrolPath->Num();
	int32 nextIndex = INDEX_NONE;

	switch (patrolMode)
	{
	case EPatrolMode::Random:
	{
		// Select Point [Random]
		nextIndex = FMath::RandRange(0, count - 1);

		// Reroll
		if (count > 1 && nextIndex == currentIndex)
			nextIndex = (nextIndex + 1) % count;

		break;
	}

	case EPatrolMode::Loop:
	{
		if (currentIndex < 0) currentIndex = 0;

		// Select Point [Loop]
		nextIndex = (currentIndex + 1) % count;

		break;
	}

	case EPatrolMode::Reverse:
	{
		if (currentIndex < 0) currentIndex = 0;

		// Select Point [Reverse]
		nextIndex = bPatrolReverse ? currentIndex - 1 : currentIndex + 1;

		// Reverse in last point (count - 1 -> count - 2)
		if (nextIndex >= count)
		{
			bPatrolReverse = true;
			nextIndex = count - 2;
		}

		// Reverse in start point (0 -> 1)
		if (nextIndex < 0)
		{
			bPatrolReverse = false;
			nextIndex = 1;
		}


		nextIndex = FMath::Clamp(nextIndex, 0, count - 1);
		
		break;
	}
	default:
		return EBTNodeResult::Failed;
	}

	FPatrolPointData nextPatrolPointData;
	if (!patrolPath->GetPointData(nextIndex, nextPatrolPointData)) return EBTNodeResult::Failed;

	blackboardComp->SetValueAsBool(CAIKey::Patrol::bPatrolReverse, bPatrolReverse);
	blackboardComp->SetValueAsInt(CAIKey::Patrol::PatrolIndex, nextIndex);
	blackboardComp->SetValueAsVector(CAIKey::Patrol::PatrolLocation, nextPatrolPointData.Location);

	return EBTNodeResult::Succeeded;
}