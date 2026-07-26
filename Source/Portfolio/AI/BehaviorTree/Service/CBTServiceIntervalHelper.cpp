#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"

#include "AI/RuntimeLOD/CAIRuntimeLODTierResolver.h"
#include "Controller/CAIController.h"
#include "Core/Profiling/CAIBehaviorTreeProfiling.h"
#include "Core/Profiling/CAIStateRuntimeLODProfiling.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "HAL/IConsoleManager.h"

namespace
{
	enum class EBTServiceIntervalMode : int32
	{
		Default = 0,
		Reduced = 1,
		AggressiveReduced = 2,
	};

	constexpr int32 ToBTServiceIntervalModeValue(EBTServiceIntervalMode InMode)
	{
		return static_cast<int32>(InMode);
	}

	// Console Variable
	TAutoConsoleVariable<int32> CVarBTUpdateIntervalMode(
		TEXT("Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode"),
		ToBTServiceIntervalModeValue(EBTServiceIntervalMode::Default),
		TEXT("Controls AI BT service update interval Runtime LOD mode. 0: default, 1: reduced, 2: aggressive reduced."),
		ECVF_Default);

	// Interval Defaults
	constexpr float DefaultAIContextInterval = 0.1f;
	constexpr float DefaultAIIntentStateInterval = 0.2f;
	constexpr float ReducedAIIntentStateInterval = 0.3f;
	constexpr float AggressiveAIIntentStateInterval = 0.5f;
	constexpr float DefaultEngageContextInterval = 0.1f;
	constexpr float DefaultInvestigateContextInterval = 0.1f;
	constexpr float DefaultRandomDeviation = 0.0f;

	// Mode Query
	EBTServiceIntervalMode GetBTUpdateIntervalMode()
	{
		const int32 modeValue = FMath::Clamp(
			CVarBTUpdateIntervalMode.GetValueOnGameThread(),
			ToBTServiceIntervalModeValue(EBTServiceIntervalMode::Default),
			ToBTServiceIntervalModeValue(EBTServiceIntervalMode::AggressiveReduced));

		return static_cast<EBTServiceIntervalMode>(modeValue);
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

	// Interval Preset Selection
	EBTServiceIntervalPreset SelectIntervalPreset(EBTServiceIntervalMode InMode, EAIRuntimeLODTier InTier)
	{
		switch (InMode)
		{
		case EBTServiceIntervalMode::Default:
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

		case EBTServiceIntervalMode::Reduced:
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

		case EBTServiceIntervalMode::AggressiveReduced:
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
		FAIStateRuntimeLODProfiling::RecordResolvedTier(InTier);
	}

	// Profiling
	void RecordAIIntentStateIntervalPreset(EBTServiceIntervalPreset InPreset)
	{
		FAIBehaviorTreeProfiling::RecordAIIntentIntervalPreset(InPreset);
	}

	// Interval Value
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

	// Interval Selection
	float SelectAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp)
	{
		const EAIRuntimeLODTier runtimeLODTier = GetRuntimeLODTierForIntervalSelection(InOwnerComp);
		const EBTServiceIntervalPreset intervalPreset = SelectIntervalPreset(
			GetBTUpdateIntervalMode(),
			runtimeLODTier);

		// Record the selected Runtime LOD tier and interval preset.
		RecordStateRuntimeLODTier(runtimeLODTier);
		RecordAIIntentStateIntervalPreset(intervalPreset);

		return GetAIIntentStateIntervalByPreset(intervalPreset);
	}
}

// Public API

float CBTServiceIntervalHelper::GetDefaultAIContextInterval()
{
	return DefaultAIContextInterval;
}

float CBTServiceIntervalHelper::GetDefaultAIIntentStateInterval()
{
	return DefaultAIIntentStateInterval;
}

float CBTServiceIntervalHelper::GetDefaultEngageContextInterval()
{
	return DefaultEngageContextInterval;
}

float CBTServiceIntervalHelper::GetDefaultInvestigateContextInterval()
{
	return DefaultInvestigateContextInterval;
}

float CBTServiceIntervalHelper::GetDefaultRandomDeviation()
{
	return DefaultRandomDeviation;
}

float CBTServiceIntervalHelper::GetAIContextInterval(const UBehaviorTreeComponent& /*InOwnerComp*/)
{
	return GetDefaultAIContextInterval();
}

float CBTServiceIntervalHelper::GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp)
{
	return SelectAIIntentStateInterval(InOwnerComp);
}

float CBTServiceIntervalHelper::GetEngageContextInterval()
{
	return GetDefaultEngageContextInterval();
}
