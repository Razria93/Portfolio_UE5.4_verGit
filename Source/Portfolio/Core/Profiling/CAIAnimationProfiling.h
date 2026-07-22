#pragma once

#include "CoreMinimal.h"

class FAIAnimationProfiling
{
public:
	// Gate
	static bool ShouldAuditAnimationRefresh();

	// Counter
	static void RecordAnimationRefreshAttempt();
	static void RecordAnimationRefreshExecuted();
	static void RecordAnimationRefreshSkipped();
};
