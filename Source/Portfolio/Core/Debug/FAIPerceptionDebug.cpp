#include "Core/Debug/FAIPerceptionDebug.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarPerceptionCandidateAudit(
		TEXT("Portfolio.AI.RuntimeLOD.PerceptionCandidateAudit"),
		0,
		TEXT("Enable Enemy Perception candidate audit for runtime LOD measurement. 0: disabled, 1: enabled."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarBlackboardEngageLatencyAudit(
		TEXT("Portfolio.AI.RuntimeLOD.BlackboardEngageLatencyAudit"),
		0,
		TEXT("Enable Enemy Blackboard / Engage latency audit for runtime LOD measurement. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif
}

// Gate

bool FAIPerceptionDebug::ShouldAuditPerceptionCandidates()
{
#if !UE_BUILD_SHIPPING
	return CVarPerceptionCandidateAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

bool FAIPerceptionDebug::ShouldAuditBlackboardEngageLatency()
{
#if !UE_BUILD_SHIPPING
	return CVarBlackboardEngageLatencyAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}
