#include "AI/Patrol/CPatrolPath.h"
#include "AI/Patrol/CPatrolPoint.h"

ACPatrolPath::ACPatrolPath()
{
}

bool ACPatrolPath::GetPointData(int32 InIndex, FPatrolPointData& OutPatrolPointData) const
{
	if (!PatrolPoints.IsValidIndex(InIndex)) return false;

	const ACPatrolPoint* point = PatrolPoints[InIndex];
	if (!IsValid(point)) return false;

	OutPatrolPointData.Location = point->GetActorLocation();
	OutPatrolPointData.ExtraWaitTime = point->ExtraWaitTime;
	OutPatrolPointData.bFaceOnArrive = point->bFaceOnArrive;
	OutPatrolPointData.FaceYaw = point->FaceYaw;
	OutPatrolPointData.PointTag = point->PointTag;

	return true;
}
