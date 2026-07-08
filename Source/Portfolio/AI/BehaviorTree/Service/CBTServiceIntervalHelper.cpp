#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "HAL/IConsoleManager.h"
#include "ProfilingDebugging/CsvProfiler.h"

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

	// Mode + Precision -> Interval Enum Preset
	EBTServiceIntervalPreset SelectAIIntentStateIntervalPreset(int32 InMode, EAIUpdatePrecision InPrecision)
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

	// Interval Enum Preset -> Interval float value (return)
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

	// Profiling Counter
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

	// Interval Select
	float SelectAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp)
	{
		const EBTServiceIntervalPreset intervalPreset = SelectAIIntentStateIntervalPreset(
			GetBTUpdateIntervalMode(),
			ResolveAIUpdatePrecision(InOwnerComp));

		// Profiling
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
