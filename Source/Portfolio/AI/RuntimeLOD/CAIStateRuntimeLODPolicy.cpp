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

	EAIStateRuntimeLODTier ResolveTierFromIntentState(EAIIntentState InAIIntentState, bool bHasTargetAwareness)
	{
		switch (InAIIntentState)
		{
		case EAIIntentState::Dead:
		case EAIIntentState::HitReact:
		case EAIIntentState::Engage:
			return EAIStateRuntimeLODTier::Engage;

		case EAIIntentState::Alert:
		case EAIIntentState::Chase:
		case EAIIntentState::Investigate:
			return EAIStateRuntimeLODTier::Alert;

		case EAIIntentState::Observe:
			return EAIStateRuntimeLODTier::Observe;

		case EAIIntentState::Patrol:
		case EAIIntentState::Idle:
			return EAIStateRuntimeLODTier::Idle;

		case EAIIntentState::Max:
		default:
			return bHasTargetAwareness ? EAIStateRuntimeLODTier::Observe : EAIStateRuntimeLODTier::Idle;
		}
	}
}

int32 FAIStateRuntimeLODPolicy::GetStatePolicyMode()
{
	return FMath::Clamp(CVarAIStateRuntimeLODPolicyMode.GetValueOnGameThread(), 0, 1);
}

bool FAIStateRuntimeLODPolicy::IsStatePolicyEnabled()
{
	return GetStatePolicyMode() > 0;
}

EAIStateRuntimeLODTier FAIStateRuntimeLODPolicy::ResolveTier(const FAIStateRuntimeLODContext& InContext)
{
	if (InContext.bDormantCandidate) return EAIStateRuntimeLODTier::Dormant;

	switch (InContext.CombatRole)
	{
	case ECombatRole::Engage:
		return EAIStateRuntimeLODTier::Engage;

	case ECombatRole::Alert:
		return EAIStateRuntimeLODTier::Alert;

	case ECombatRole::None:
	default:
		return ResolveTierFromIntentState(InContext.AIIntentState, InContext.bHasTargetAwareness);
	}
}

void FAIStateRuntimeLODPolicy::RecordResolvedTierForProfiling(EAIStateRuntimeLODTier InTier)
{
	if (!IsStatePolicyEnabled()) return;

	switch (InTier)
	{
	case EAIStateRuntimeLODTier::Engage:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Engage_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIStateRuntimeLODTier::Alert:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Alert_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIStateRuntimeLODTier::Observe:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Observe_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIStateRuntimeLODTier::Idle:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Idle_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EAIStateRuntimeLODTier::Dormant:
	default:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_StateLOD_Tier_Dormant_Count, 1, ECsvCustomStatOp::Accumulate);
		return;
	}
}

const TCHAR* FAIStateRuntimeLODPolicy::LexToString(EAIStateRuntimeLODTier InTier)
{
	switch (InTier)
	{
	case EAIStateRuntimeLODTier::Engage:
		return TEXT("Engage");

	case EAIStateRuntimeLODTier::Alert:
		return TEXT("Alert");

	case EAIStateRuntimeLODTier::Observe:
		return TEXT("Observe");

	case EAIStateRuntimeLODTier::Idle:
		return TEXT("Idle");

	case EAIStateRuntimeLODTier::Dormant:
	default:
		return TEXT("Dormant");
	}
}
