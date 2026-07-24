#include "AI/Patrol/CPatrolPath.h"

#include "AI/Patrol/CPatrolPoint.h"

ACPatrolPath::ACPatrolPath()
{
}

bool ACPatrolPath::GetPointSnapshot(int32 InIndex, FPatrolPointSnapshot& OutPatrolPointSnapshot) const
{
	if (!PatrolPoints.IsValidIndex(InIndex)) return false;

	const ACPatrolPoint* point = PatrolPoints[InIndex];
	if (!IsValid(point)) return false;

	OutPatrolPointSnapshot.Location = point->GetActorLocation();
	OutPatrolPointSnapshot.ExtraWaitTime = point->ExtraWaitTime;
	OutPatrolPointSnapshot.bFaceOnArrive = point->bFaceOnArrive;
	OutPatrolPointSnapshot.FaceYaw = point->FaceYaw;
	OutPatrolPointSnapshot.PointTag = point->PointTag;

	return true;
}
