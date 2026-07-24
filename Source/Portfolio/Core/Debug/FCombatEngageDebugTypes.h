#pragma once

#include "CoreMinimal.h"

struct FEngageAssignmentRebuildDebugState
{
	int32 RequestSnapshotCount = 0;
	int32 RequestBucketCount = 0;
	int32 WarmupRequestCount = 0;
	int32 FreshAppliedCount = 0;
	int32 PromotedCount = 0;
	int32 PreservedEngageCount = 0;
	int32 PreservedAlertCount = 0;
};
