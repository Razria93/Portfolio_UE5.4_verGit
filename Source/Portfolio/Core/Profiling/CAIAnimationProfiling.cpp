#include "Core/Profiling/CAIAnimationProfiling.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

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

void FAIAnimationProfiling::RecordAnimationRefreshAttemptForProfiling()
{
	if (!ShouldAuditAnimationRefresh()) return;

	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_AnimRefresh_Attempt, 1, ECsvCustomStatOp::Accumulate);
}

void FAIAnimationProfiling::RecordAnimationRefreshExecutedForProfiling()
{
	if (!ShouldAuditAnimationRefresh()) return;

	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_AnimRefresh_Executed, 1, ECsvCustomStatOp::Accumulate);
}

void FAIAnimationProfiling::RecordAnimationRefreshSkippedForProfiling()
{
	if (!ShouldAuditAnimationRefresh()) return;

	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_AnimRefresh_Skipped, 1, ECsvCustomStatOp::Accumulate);
}
