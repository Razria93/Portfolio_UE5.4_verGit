#pragma once

#include "CoreMinimal.h"
#include "Type/CAIStructure.h"

class PORTFOLIO_API FAIPerceptionDebug
{
public:
	// Gate
	static bool ShouldAuditPerceptionCandidates();
	static bool ShouldAuditBlackboardEngageLatency();

	// Profiling Audit
	static void PrintPerceptionCandidateAuditSummary(const AActor* InOwnerActor, const FPerceptionCandidateAuditState& InState);
	static void PrintBlackboardEngageLatencyAuditSummary(const AActor* InOwnerActor, const FBlackboardEngageLatencyAuditState& InState);
};
