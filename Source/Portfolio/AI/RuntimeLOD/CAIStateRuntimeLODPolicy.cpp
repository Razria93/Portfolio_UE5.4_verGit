#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

namespace
{
	TAutoConsoleVariable<int32> CVarAIStateRuntimeLODPolicyMode(
		TEXT("Portfolio.AI.RuntimeLOD.StatePolicyMode"),
		0,
		TEXT("Controls state-based AI Runtime LOD policy. 0: disabled, 1: conservative policy audit."),
		ECVF_Default);
}

int32 FAIStateRuntimeLODPolicy::GetStatePolicyMode()
{
	return FMath::Clamp(CVarAIStateRuntimeLODPolicyMode.GetValueOnGameThread(), 0, 1);
}

bool FAIStateRuntimeLODPolicy::IsStatePolicyAuditEnabled()
{
	return GetStatePolicyMode() > 0;
}

void FAIStateRuntimeLODPolicy::RecordResolvedTierForProfiling(EAIRuntimeLODTier InTier)
{
	if (!IsStatePolicyAuditEnabled()) return;

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
