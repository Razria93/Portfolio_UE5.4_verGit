#pragma once

#include "CoreMinimal.h"

class PORTFOLIO_API FAIPerceptionDebug
{
public:
	// Gate
	static bool ShouldAuditPerceptionCandidates();
	static bool ShouldAuditBlackboardEngageLatency();
};
