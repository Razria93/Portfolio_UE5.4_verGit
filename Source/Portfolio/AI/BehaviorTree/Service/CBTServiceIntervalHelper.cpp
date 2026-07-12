#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"
#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"
#include "Controller/CAIController.h"

namespace
{
	// Console Variable
	TAutoConsoleVariable<int32> CVarBTUpdateIntervalMode(
		TEXT("Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode"),
		0,
		TEXT("Controls AI BT service update interval profiling mode. 0: default, 1: reduced, 2: aggressive reduced."),
		ECVF_Default);

	// Interval Preset
	enum class EBTServiceIntervalPreset : uint8
	{
		Default,
		Reduced,
		Aggressive
	};

	// AIContext
	constexpr float DefaultAIContextInterval = 0.1f;

	// AIIntentState
	constexpr float DefaultAIIntentStateInterval = 0.2f;
	constexpr float ReducedAIIntentStateInterval = 0.3f;
	constexpr float AggressiveAIIntentStateInterval = 0.5f;

	// EngageContext
	constexpr float DefaultEngageContextInterval = 0.1f;

	// Mode Query
	int32 GetBTUpdateIntervalMode()
	{
		return FMath::Clamp(CVarBTUpdateIntervalMode.GetValueOnGameThread(), 0, 2);
	}

	// Runtime LOD Tier Snapshot
	const ACAIController* GetAIController(const UBehaviorTreeComponent& InOwnerComp)
	{
		return Cast<ACAIController>(InOwnerComp.GetAIOwner());
	}

	EAIRuntimeLODTier GetRuntimeLODTierSnapshot(const ACAIController& InAIController)
	{
		return InAIController.GetCurrentRuntimeLODTier();
	}

	EAIRuntimeLODTier ResolveRuntimeLODTierFallback(const UBehaviorTreeComponent& InOwnerComp)
	{
		const UBlackboardComponent* blackboardComp = InOwnerComp.GetBlackboardComponent();
		if (!IsValid(blackboardComp)) return EAIRuntimeLODTier::CombatCritical;

		return FAIRuntimeLODTierResolver::ResolveTier(*blackboardComp);
	}

	EAIRuntimeLODTier GetRuntimeLODTierForIntervalSelection(const UBehaviorTreeComponent& InOwnerComp)
	{
		const ACAIController* aiController = GetAIController(InOwnerComp);
		if (IsValid(aiController)) return GetRuntimeLODTierSnapshot(*aiController);

		return ResolveRuntimeLODTierFallback(InOwnerComp);
	}

	// Mode + Runtime LOD Tier -> Interval Enum Preset
	EBTServiceIntervalPreset SelectIntervalPreset(int32 InMode, EAIRuntimeLODTier InTier)
	{
		switch (InMode)
		{
		case 0:
			switch (InTier)
			{
			case EAIRuntimeLODTier::CombatCritical:
			case EAIRuntimeLODTier::CombatSupport:
			case EAIRuntimeLODTier::Awareness:
			case EAIRuntimeLODTier::Background:
			case EAIRuntimeLODTier::Dormant:
			default:
				return EBTServiceIntervalPreset::Default;
			}

		case 1:
			switch (InTier)
			{
			case EAIRuntimeLODTier::CombatCritical:
				return EBTServiceIntervalPreset::Default;

			case EAIRuntimeLODTier::CombatSupport:
			case EAIRuntimeLODTier::Awareness:
			case EAIRuntimeLODTier::Background:
			case EAIRuntimeLODTier::Dormant:
			default:
				return EBTServiceIntervalPreset::Reduced;
			}

		case 2:
		default:
			switch (InTier)
			{
			case EAIRuntimeLODTier::CombatCritical:
				return EBTServiceIntervalPreset::Default;

			case EAIRuntimeLODTier::CombatSupport:
				return EBTServiceIntervalPreset::Reduced;

			case EAIRuntimeLODTier::Awareness:
			case EAIRuntimeLODTier::Background:
			case EAIRuntimeLODTier::Dormant:
			default:
				return EBTServiceIntervalPreset::Aggressive;
			}
		}
	}

	void RecordStateRuntimeLODTier(EAIRuntimeLODTier InTier)
	{
		if (!FAIStateRuntimeLODPolicy::IsStatePolicyAuditEnabled()) return;

		FAIStateRuntimeLODPolicy::RecordResolvedTierForProfiling(InTier);
	}

	// AIIntentState
	// Record Counter (AIIntentState)
	void RecordAIIntentStateIntervalPreset(EBTServiceIntervalPreset InPreset)
	{
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
	}

	// Interval Enum Preset -> Interval float value (AIIntentState)
	float GetAIIntentStateIntervalByPreset(EBTServiceIntervalPreset InPreset)
	{
		switch (InPreset)
		{
		case EBTServiceIntervalPreset::Default:
			return DefaultAIIntentStateInterval;

		case EBTServiceIntervalPreset::Reduced:
			return ReducedAIIntentStateInterval;

		case EBTServiceIntervalPreset::Aggressive:
		default:
			return AggressiveAIIntentStateInterval;
		}
	}

	// Interval Select (AIIntentState)
	float SelectAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp)
	{
		const EAIRuntimeLODTier runtimeLODTier = GetRuntimeLODTierForIntervalSelection(InOwnerComp);
		const EBTServiceIntervalPreset intervalPreset = SelectIntervalPreset(
			GetBTUpdateIntervalMode(),
			runtimeLODTier);

		// Profiling
		RecordStateRuntimeLODTier(runtimeLODTier);
		RecordAIIntentStateIntervalPreset(intervalPreset);

		// return Interval float value
		return GetAIIntentStateIntervalByPreset(intervalPreset);
	}
}

float CBTServiceIntervalHelper::GetAIContextInterval(const UBehaviorTreeComponent& /*InOwnerComp*/)
{
	return DefaultAIContextInterval;
}

float CBTServiceIntervalHelper::GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp)
{
	return SelectAIIntentStateInterval(InOwnerComp);
}

float CBTServiceIntervalHelper::GetEngageContextInterval()
{
	return DefaultEngageContextInterval;
}
