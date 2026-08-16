#pragma once

#include "CoreMinimal.h"

struct FEngageAssignmentRebuildDebugState
{
	int32 CandidateCount = 0;
	int32 PreservedEngageCount = 0;
	int32 PreservedAlertCount = 0;
	int32 PreservedObserveCount = 0;
	int32 PromotedAlertToEngageCount = 0;
	int32 PromotedObserveToEngageCount = 0;
	int32 PromotedObserveToAlertCount = 0;
	int32 FreshEngageCount = 0;
	int32 FreshAlertCount = 0;
	int32 FreshObserveCount = 0;
};
