#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

#include "AI/Blackboard/CAIKey.h"
#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"
#include "Controller/CAIController.h"
#include "System/Combat/CWorldSubsystem_CombatEngage.h"
#include "Type/CWorldSubSystemStructure.h"

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
	constexpr float ReducedAIContextInterval = 0.2f;
	constexpr float AggressiveAIContextInterval = 0.4f;

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

	// Precision Resolve
	EAIUpdatePrecision ResolveAIUpdatePrecision(const UBehaviorTreeComponent& InOwnerComp)
	{
		const ACAIController* aiController = Cast<ACAIController>(InOwnerComp.GetAIOwner());
		if (!IsValid(aiController)) return EAIUpdatePrecision::High;

		UWorld* world = aiController->GetWorld();
		if (!IsValid(world)) return EAIUpdatePrecision::High;

		const UCWorldSubsystem_CombatEngage* engageSubsystem = world->GetSubsystem<UCWorldSubsystem_CombatEngage>();
		if (!IsValid(engageSubsystem)) return EAIUpdatePrecision::High;

		return engageSubsystem->GetAIUpdatePrecision(aiController);
	}

	// State Runtime LOD Audit
	FAIStateRuntimeLODContext BuildStateRuntimeLODContext(const UBlackboardComponent& InBlackboardComp)
	{
		FAIStateRuntimeLODContext context;
		context.AIIntentState = static_cast<EAIIntentState>(InBlackboardComp.GetValueAsEnum(CAIKey::State::AIIntentState.KeyName));
		context.CombatRole = static_cast<ECombatRole>(InBlackboardComp.GetValueAsEnum(CAIKey::Engage::CombatRole.KeyName));

		const AActor* targetActor = Cast<AActor>(InBlackboardComp.GetValueAsObject(CAIKey::Targeting::TargetActor.KeyName));
		const bool bHasLOS = InBlackboardComp.GetValueAsBool(CAIKey::Perception::bHasLOS.KeyName);
		context.bHasTargetAwareness = IsValid(targetActor) || bHasLOS;

		return context;
	}

	void RecordStateRuntimeLODTier(const UBehaviorTreeComponent& InOwnerComp)
	{
		if (!FAIStateRuntimeLODPolicy::IsStatePolicyEnabled()) return;

		const UBlackboardComponent* blackboardComp = InOwnerComp.GetBlackboardComponent();
		if (!IsValid(blackboardComp)) return;

		const EAIStateRuntimeLODTier tier = FAIStateRuntimeLODPolicy::ResolveTier(BuildStateRuntimeLODContext(*blackboardComp));
		FAIStateRuntimeLODPolicy::RecordResolvedTierForProfiling(tier);
	}

	// Mode + Precision -> Interval Enum Preset
	EBTServiceIntervalPreset SelectIntervalPreset(int32 InMode, EAIUpdatePrecision InPrecision)
	{
		switch (InMode)
		{
		case 0:
			switch (InPrecision)
			{
			case EAIUpdatePrecision::High:
			case EAIUpdatePrecision::Reduced:
			case EAIUpdatePrecision::Low:
			default:
				return EBTServiceIntervalPreset::Default;
			}

		case 1:
			switch (InPrecision)
			{
			case EAIUpdatePrecision::High:
				return EBTServiceIntervalPreset::Default;

			case EAIUpdatePrecision::Reduced:
			case EAIUpdatePrecision::Low:
			default:
				return EBTServiceIntervalPreset::Reduced;
			}

		case 2:
		default:
			switch (InPrecision)
			{
			case EAIUpdatePrecision::High:
				return EBTServiceIntervalPreset::Default;

			case EAIUpdatePrecision::Reduced:
				return EBTServiceIntervalPreset::Reduced;

			case EAIUpdatePrecision::Low:
			default:
				return EBTServiceIntervalPreset::Aggressive;
			}
		}
	}

	// AIContext
	// Record Counter (AIContext)
	void RecordAIContextIntervalPreset(EBTServiceIntervalPreset InPreset)
	{
		switch (InPreset)
		{
		case EBTServiceIntervalPreset::Default:
			CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_AIContextInterval_Default_Count, 1, ECsvCustomStatOp::Accumulate);
			return;

		case EBTServiceIntervalPreset::Reduced:
			CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_AIContextInterval_Reduced_Count, 1, ECsvCustomStatOp::Accumulate);
			return;

		case EBTServiceIntervalPreset::Aggressive:
		default:
			CSV_CUSTOM_STAT_GLOBAL(PortfolioAI_BT_AIContextInterval_Aggressive_Count, 1, ECsvCustomStatOp::Accumulate);
			return;
		}
	}

	// Interval Enum Preset -> Interval float value (AIContext)
	float GetAIContextIntervalByPreset(EBTServiceIntervalPreset InPreset)
	{
		switch (InPreset)
		{
		case EBTServiceIntervalPreset::Default:
			return DefaultAIContextInterval;

		case EBTServiceIntervalPreset::Reduced:
			return ReducedAIContextInterval;

		case EBTServiceIntervalPreset::Aggressive:
		default:
			return AggressiveAIContextInterval;
		}
	}

	// Interval Select (AIContext)
	float SelectAIContextInterval(const UBehaviorTreeComponent& InOwnerComp)
	{
		const EBTServiceIntervalPreset intervalPreset = SelectIntervalPreset(
			GetBTUpdateIntervalMode(),
			ResolveAIUpdatePrecision(InOwnerComp));

		// Profiling
		RecordAIContextIntervalPreset(intervalPreset);

		// return Interval float value
		return GetAIContextIntervalByPreset(intervalPreset);
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
		const EBTServiceIntervalPreset intervalPreset = SelectIntervalPreset(
			GetBTUpdateIntervalMode(),
			ResolveAIUpdatePrecision(InOwnerComp));

		// Profiling
		RecordAIIntentStateIntervalPreset(intervalPreset);
		RecordStateRuntimeLODTier(InOwnerComp);

		// return Interval float value
		return GetAIIntentStateIntervalByPreset(intervalPreset);
	}
}

float CBTServiceIntervalHelper::GetAIContextInterval(const UBehaviorTreeComponent& InOwnerComp)
{
	return SelectAIContextInterval(InOwnerComp);
}

float CBTServiceIntervalHelper::GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp)
{
	return SelectAIIntentStateInterval(InOwnerComp);
}

float CBTServiceIntervalHelper::GetEngageContextInterval()
{
	return DefaultEngageContextInterval;
}
