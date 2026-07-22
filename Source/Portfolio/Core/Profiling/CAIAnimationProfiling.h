#pragma once

#include "CoreMinimal.h"

class FAIAnimationProfiling
{
public:
	// Gate
	static bool ShouldAuditAnimationRefresh();

	// Counter
	static void RecordAnimationRefreshAttemptForProfiling();
	static void RecordAnimationRefreshExecutedForProfiling();
	static void RecordAnimationRefreshSkippedForProfiling();
};
