#include "AI/BehaviorTree/Task/CBTTask_SelectPatrolPoint.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Patrol/CPatrolPath.h"

#include "Type/CAITypes.h"
#include "AI/Blackboard/CAIKey.h"

UCBTTask_SelectPatrolPoint::UCBTTask_SelectPatrolPoint()
{
	NodeName = TEXT("Select Patrol Point");
}

EBTNodeResult::Type UCBTTask_SelectPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* blackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return EBTNodeResult::Failed;

	const AAIController* aiOwner = OwnerComp.GetAIOwner();
	APawn* ownerPawn = IsValid(aiOwner) ? aiOwner->GetPawn() : nullptr;
	if (!IsValid(ownerPawn)) return EBTNodeResult::Failed;

	bool bUsePatrol = blackboardComp->GetValueAsBool(CAIKey::Patrol::bUsePatrol.KeyName);
	ACPatrolPath* patrolPath = Cast<ACPatrolPath>(blackboardComp->GetValueAsObject(CAIKey::Patrol::PatrolPath.KeyName));
	EPatrolMode patrolMode = static_cast<EPatrolMode>(blackboardComp->GetValueAsEnum(CAIKey::Patrol::PatrolMode.KeyName));

	if (!bUsePatrol || !IsValid(patrolPath) || patrolPath->Num() <= 0 || patrolMode == EPatrolMode::None) return EBTNodeResult::Failed;
	
	int32 currentIndex = blackboardComp->GetValueAsInt(CAIKey::Patrol::PatrolIndex.KeyName);
	bool bPatrolReverse = blackboardComp->GetValueAsBool(CAIKey::Patrol::bPatrolReverse.KeyName);
	
	const int32 count = patrolPath->Num();
	int32 nextIndex = INDEX_NONE;

	switch (patrolMode)
	{
	case EPatrolMode::Random:
	{
		nextIndex = FMath::RandRange(0, count - 1);

		// Reroll
		if (count > 1 && nextIndex == currentIndex)
			nextIndex = (nextIndex + 1) % count;

		break;
	}

	case EPatrolMode::Loop:
	{
		if (currentIndex < 0) currentIndex = 0;

		nextIndex = (currentIndex + 1) % count;

		break;
	}

	case EPatrolMode::Reverse:
	{
		if (currentIndex < 0) currentIndex = 0;

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

	FPatrolPointSnapshot nextPatrolPointSnapshot;
	if (!patrolPath->GetPointSnapshot(nextIndex, nextPatrolPointSnapshot)) return EBTNodeResult::Failed;

	blackboardComp->SetValueAsBool(CAIKey::Patrol::bPatrolReverse.KeyName, bPatrolReverse);
	blackboardComp->SetValueAsInt(CAIKey::Patrol::PatrolIndex.KeyName, nextIndex);
	blackboardComp->SetValueAsVector(CAIKey::Patrol::PatrolLocation.KeyName, nextPatrolPointSnapshot.Location);

	return EBTNodeResult::Succeeded;
}
