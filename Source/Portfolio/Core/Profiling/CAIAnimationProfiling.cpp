#include "Core/Profiling/CAIAnimationProfiling.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarAnimationRefreshAudit(
		TEXT("Portfolio.AI.RuntimeLOD.AnimationRefreshAudit"),
		0,
		TEXT("Emit ACEnemy animation parameter refresh CSV counters. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif
}

bool FAIAnimationProfiling::ShouldAuditAnimationRefresh()
{
#if !UE_BUILD_SHIPPING
	return CVarAnimationRefreshAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}
