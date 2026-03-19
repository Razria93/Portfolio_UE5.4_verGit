#include "AI/BehaviorTree/Service/CBTService_UpdatePatrolContext.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AI/Patrol/CPatrolPath.h"

#include "AI/BlackBoard/CAIKey.h"
#include "Type/CAIStructure.h"

UCBTService_UpdatePatrolContext::UCBTService_UpdatePatrolContext()
{
	NodeName = "Update Patrol Context";
	bNotifyTick = true;

	Interval = 0.1f;
	RandomDeviation = 0.0f;
}

void UCBTService_UpdatePatrolContext::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* blackBoardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(blackBoardComp)) return;

	APawn* ownerPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!IsValid(ownerPawn))
	{
		ClearPatrolContext(blackBoardComp);
		return;
	}

	FPatrolContext patrolContext; // OutParameter
	const EContextBuildResult buildResult = BuildPatrolContext(ownerPawn, blackBoardComp, patrolContext);

	if (buildResult != EContextBuildResult::Success)
	{
		ClearPatrolContext(blackBoardComp);
		return;
	}

	const EContextBuildResult computeResult = ComputePatrolContext(ownerPawn, blackBoardComp, patrolContext);

	if (computeResult != EContextBuildResult::Success)
	{
		ClearPatrolContext(blackBoardComp);
		return;
	}

	UpdatePatrolContext(blackBoardComp, patrolContext);
}

EContextBuildResult UCBTService_UpdatePatrolContext::BuildPatrolContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FPatrolContext& OutPatrolContext)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return EContextBuildResult::Error;

	const bool bUsePatrol = InBlackboardComp->GetValueAsBool(CAIKey::Patrol::bUsePatrol);
	if (!bUsePatrol) return EContextBuildResult::NoData;

	OutPatrolContext.bUsePatrol = InBlackboardComp->GetValueAsBool(CAIKey::Patrol::bUsePatrol);
	OutPatrolContext.PatrolPath = Cast<ACPatrolPath>(InBlackboardComp->GetValueAsObject(CAIKey::Patrol::PatrolPath));
	OutPatrolContext.PatrolMode = static_cast<EPatrolMode>(InBlackboardComp->GetValueAsEnum(CAIKey::Patrol::PatrolMode));

	if (!OutPatrolContext.bUsePatrol || !IsValid(OutPatrolContext.PatrolPath) || OutPatrolContext.PatrolPath->Num() <= 0 || OutPatrolContext.PatrolMode == EPatrolMode::None)
		return EContextBuildResult::NoData;

	OutPatrolContext.CurrentIndex = InBlackboardComp->GetValueAsInt(CAIKey::Patrol::PatrolIndex);
	OutPatrolContext.bPatrolReverse = InBlackboardComp->GetValueAsBool(CAIKey::Patrol::bPatrolReverse);
	OutPatrolContext.CurrentPatrolLocation = InBlackboardComp->GetValueAsVector(CAIKey::Patrol::PatrolLocation);

	return EContextBuildResult::Success;
}

EContextBuildResult UCBTService_UpdatePatrolContext::ComputePatrolContext(APawn* InOwnerPawn, UBlackboardComponent* InBlackboardComp, FPatrolContext& InOutPatrolContext)
{
	const bool bHasCurrentTarget = (InOutPatrolContext.CurrentIndex >= 0);
	const bool bReached = bHasCurrentTarget ? IsReached(InOwnerPawn->GetActorLocation(), InOutPatrolContext.CurrentPatrolLocation) : true;

	if (!bReached)
	{
		InOutPatrolContext.NextIndex = InOutPatrolContext.CurrentIndex;
		InOutPatrolContext.NextPatrolLocation = InOutPatrolContext.CurrentPatrolLocation;

		return EContextBuildResult::Success;
	}

	int32 NextIndex = INDEX_NONE; // OutParameter
	if (!ComputeNextIndex(InOutPatrolContext.PatrolPath->Num(), InOutPatrolContext.bPatrolReverse, InOutPatrolContext.PatrolMode, InOutPatrolContext.CurrentIndex, NextIndex))
		return EContextBuildResult::NoData;

	FPatrolPointData PointData; // OutParameter
	if (!InOutPatrolContext.PatrolPath->GetPointData(NextIndex, PointData))
		return EContextBuildResult::NoData;

	InOutPatrolContext.NextIndex = NextIndex;
	InOutPatrolContext.NextPatrolLocation = PointData.Location;
	// TODO: add PatrolPointData into PatrolContext

	PrintPatrolContextData(InOwnerPawn, InBlackboardComp);

	return EContextBuildResult::Success;
}

bool UCBTService_UpdatePatrolContext::ComputeNextIndex(int32 InCount, bool& InOutPatrolReverse, EPatrolMode InPatrolMode, int32 InCurrentIndex, int32& OutNextIndex)
{
	if (InCount <= 0) return false;

	switch (InPatrolMode)
	{
	case EPatrolMode::Random:
	{
		// Random
		int32 nextIndex = FMath::RandRange(0, InCount - 1);

		// Reroll
		if (InCount > 1 && nextIndex == InCurrentIndex)
			nextIndex = (nextIndex + 1) % InCount;

		OutNextIndex = nextIndex;

		return true;
	}
	case EPatrolMode::Loop:
	{
		// Init
		if (InCurrentIndex < 0) InCurrentIndex = -1;

		// Loop
		OutNextIndex = (InCurrentIndex + 1) % InCount;

		return true;
	}
	case EPatrolMode::Reverse:
	{
		// Init
		if (InCurrentIndex < 0) InCurrentIndex = 0;

		// Progress
		int32 nextIndex = InOutPatrolReverse ? InCurrentIndex - 1 : InCurrentIndex + 1;

		// Reverse in last point (count - 1 -> count - 2)
		if (nextIndex >= InCount)
		{
			InOutPatrolReverse = true;
			nextIndex = InCount - 2;
		}

		// Reverse in start point (0 -> 1)
		if (nextIndex < 0)
		{
			InOutPatrolReverse = false;
			nextIndex = 1;
		}

		OutNextIndex = FMath::Clamp(nextIndex, 0, InCount - 1);

		return true;
	}
	default:
		return false;
	}
}

void UCBTService_UpdatePatrolContext::UpdatePatrolContext(UBlackboardComponent* InBlackboardComp, const FPatrolContext& InPatrolContext)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->SetValueAsBool(CAIKey::Patrol::bPatrolReverse, InPatrolContext.bPatrolReverse);
	InBlackboardComp->SetValueAsInt(CAIKey::Patrol::PatrolIndex, InPatrolContext.NextIndex);
	InBlackboardComp->SetValueAsVector(CAIKey::Patrol::PatrolLocation, InPatrolContext.NextPatrolLocation);
}

void UCBTService_UpdatePatrolContext::ClearPatrolContext(UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InBlackboardComp)) return;

	InBlackboardComp->ClearValue(CAIKey::Patrol::bPatrolReverse);
}

bool UCBTService_UpdatePatrolContext::IsReached(const FVector& InOwnerLocation, const FVector& InPatrolLocation) const
{
	const float dist2D = FVector::Dist2D(InOwnerLocation, InPatrolLocation);
	const float diff_Z = FMath::Abs(InOwnerLocation.Z - InPatrolLocation.Z);

	bool bReached_XY = dist2D <= ReachThreshold_XY;
	bool bReached_Z = diff_Z <= Tolerance_Z;
	bool bReached = bReached_XY && bReached_Z;

	return bReached_XY && bReached_Z;
}

void UCBTService_UpdatePatrolContext::PrintPatrolContextData(const APawn* InOwnerPawn, const UBlackboardComponent* InBlackboardComp)
{
	if (!IsValid(InOwnerPawn) || !IsValid(InBlackboardComp)) return;

	FVector ownerLocation = InOwnerPawn->GetActorLocation();
	FVector patrolLocation = InBlackboardComp->GetValueAsVector(CAIKey::Patrol::PatrolLocation);

	float dist2D = FVector::Dist2D(ownerLocation, patrolLocation);
	float diff_Z = FMath::Abs(ownerLocation.Z - patrolLocation.Z);

	bool bReached_XY = dist2D <= ReachThreshold_XY;
	bool bReached_Z = diff_Z <= Tolerance_Z;
	bool bReached = bReached_XY && bReached_Z;

	FLog::Log(FString::Printf(TEXT("[PatrolDist] Owner: %s | Patrol: %s"),
		*ownerLocation.ToCompactString(),
		*patrolLocation.ToCompactString()));

	FLog::Log(FString::Printf(TEXT("[PatrolDist] dist2D: %.2f (<= %.2f: %s) | diff_Z: %.2f (<= %.2f: %s) | Reached: %s"),
		dist2D, ReachThreshold_XY, bReached_XY ? TEXT("true") : TEXT("false"),
		diff_Z, Tolerance_Z, bReached_Z ? TEXT("true") : TEXT("false"),
		bReached ? TEXT("true") : TEXT("false")));
}