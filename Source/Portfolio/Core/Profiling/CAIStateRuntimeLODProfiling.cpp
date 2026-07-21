#include "Core/Profiling/CAIStateRuntimeLODProfiling.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarStateRuntimeLODAudit(
		TEXT("Portfolio.AI.RuntimeLOD.StatePolicyAudit"),
		0,
		TEXT("Emit AI Runtime LOD state policy tier CSV counters. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif
}

bool FAIStateRuntimeLODProfiling::ShouldAuditStateRuntimeLOD()
{
#if !UE_BUILD_SHIPPING
	return CVarStateRuntimeLODAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
}

void FAIStateRuntimeLODProfiling::RecordResolvedTierForProfiling(EAIRuntimeLODTier InTier)
{
	if (!ShouldAuditStateRuntimeLOD()) return;

	switch (InTier)
	{
	case EAIRuntimeLODTier::CombatCritical:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_CombatCritical_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIRuntimeLODTier::CombatSupport:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_CombatSupport_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIRuntimeLODTier::Awareness:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Awareness_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIRuntimeLODTier::Background:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Background_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIRuntimeLODTier::Dormant:
	default:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Dormant_Count, 1, ECsvCustomStatOp::Accumulate);
		return;
	}
}
