#include "Core/Profiling/CAIBehaviorTreeProfiling.h"

#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"
#include "ProfilingDebugging/CsvProfiler.h"

void FAIBehaviorTreeProfiling::RecordUpdateAIContextTick()
{
#if !UE_BUILD_SHIPPING
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_UpdateAIContext_Count, 1, ECsvCustomStatOp::Accumulate);
#endif
}

void FAIBehaviorTreeProfiling::RecordUpdateAIIntentStateTick()
{
#if !UE_BUILD_SHIPPING
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_UpdateAIIntentState_Count, 1, ECsvCustomStatOp::Accumulate);
#endif
}

void FAIBehaviorTreeProfiling::RecordUpdateEngageContextTick()
{
#if !UE_BUILD_SHIPPING
	CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_UpdateEngageContext_Count, 1, ECsvCustomStatOp::Accumulate);
#endif
}

void FAIBehaviorTreeProfiling::RecordAIIntentIntervalPreset(EBTServiceIntervalPreset InPreset)
{
#if !UE_BUILD_SHIPPING
	switch (InPreset)
	{
	case EBTServiceIntervalPreset::Default:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_AIIntentInterval_Default_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EBTServiceIntervalPreset::Reduced:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_AIIntentInterval_Reduced_Count, 1, ECsvCustomStatOp::Accumulate);
		return;

	case EBTServiceIntervalPreset::Aggressive:
	default:
		CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_AIIntentInterval_Aggressive_Count, 1, ECsvCustomStatOp::Accumulate);
		return;
	}
#endif
}
