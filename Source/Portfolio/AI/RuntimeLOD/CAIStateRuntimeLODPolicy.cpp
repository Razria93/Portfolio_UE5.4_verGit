#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"

#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

namespace
{
	TAutoConsoleVariable<int32> CVarAIStateRuntimeLODPolicyMode(
		TEXT("Portfolio.AI.RuntimeLOD.StatePolicyMode"),
		0,
		TEXT("Controls AI Runtime LOD policy source. 0: per-system RuntimeLOD CVar modes, 1: state-based RuntimeLOD tier snapshot."),
		ECVF_Default);

#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarAIStateRuntimeLODPolicyAudit(
		TEXT("Portfolio.AI.RuntimeLOD.StatePolicyAudit"),
		0,
		TEXT("Emit AI Runtime LOD state policy tier CSV counters. 0: disabled, 1: enabled."),
		ECVF_Default);
#endif
}

int32 FAIStateRuntimeLODPolicy::GetStatePolicyMode()
{
	return FMath::Clamp(CVarAIStateRuntimeLODPolicyMode.GetValueOnGameThread(), 0, 1);
}

bool FAIStateRuntimeLODPolicy::ShouldUseStateBasedPolicy()
{
	return GetStatePolicyMode() > 0;
}

bool FAIStateRuntimeLODPolicy::IsStatePolicyAuditEnabled()
{
#if !UE_BUILD_SHIPPING
	return CVarAIStateRuntimeLODPolicyAudit.GetValueOnGameThread() != 0;
#else
	return false;
#endif
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
