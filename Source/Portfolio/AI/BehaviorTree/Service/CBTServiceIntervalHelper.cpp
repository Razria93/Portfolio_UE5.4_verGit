#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "HAL/IConsoleManager.h"

#include "Controller/CAIController.h"
#include "System/Combat/CWorldSubsystem_CombatEngage.h"
#include "Type/CWorldSubSystemStructure.h"

namespace
{
	TAutoConsoleVariable<int32> CVarBTUpdateIntervalMode(
		TEXT("Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode"),
		0,
		TEXT("Controls AI BT service update interval profiling mode. 0: default, 1: reduced, 2: aggressive reduced."),
		ECVF_Default);

	constexpr float DefaultAIContextInterval = 0.1f;

	constexpr float DefaultAIIntentStateInterval = 0.2f;
	constexpr float ReducedAIIntentStateInterval = 0.3f;
	constexpr float AggressiveAIIntentStateInterval = 0.5f;

	constexpr float DefaultEngageContextInterval = 0.1f;

	int32 GetBTUpdateIntervalMode()
	{
		return FMath::Clamp(CVarBTUpdateIntervalMode.GetValueOnGameThread(), 0, 2);
	}

	EAIUpdatePrecision GetAIUpdatePrecision(const UBehaviorTreeComponent& InOwnerComp)
	{
		const ACAIController* aiController = Cast<ACAIController>(InOwnerComp.GetAIOwner());
		if (!IsValid(aiController)) return EAIUpdatePrecision::High;

		UWorld* world = aiController->GetWorld();
		if (!IsValid(world)) return EAIUpdatePrecision::High;

		const UCWorldSubsystem_CombatEngage* engageSubsystem = world->GetSubsystem<UCWorldSubsystem_CombatEngage>();
		if (!IsValid(engageSubsystem)) return EAIUpdatePrecision::High;

		return engageSubsystem->GetAIUpdatePrecision(aiController);
	}

	float SelectContextInterval(const UBehaviorTreeComponent& InOwnerComp, float InDefaultInterval, float InReducedInterval, float InAggressiveInterval)
	{
		const int32 mode = GetBTUpdateIntervalMode();
		if (mode == 0) return InDefaultInterval;

		switch (GetAIUpdatePrecision(InOwnerComp))
		{
		case EAIUpdatePrecision::High:
			return InDefaultInterval;

		case EAIUpdatePrecision::Reduced:
			return InReducedInterval;

		case EAIUpdatePrecision::Low:
		default:
			return InAggressiveInterval;
		}
	}
}

float CBTServiceIntervalHelper::GetAIContextInterval(const UBehaviorTreeComponent& /*InOwnerComp*/)
{
	return DefaultAIContextInterval;
}

float CBTServiceIntervalHelper::GetAIIntentStateInterval(const UBehaviorTreeComponent& InOwnerComp)
{
	return SelectContextInterval(InOwnerComp, DefaultAIIntentStateInterval, ReducedAIIntentStateInterval, AggressiveAIIntentStateInterval);
}

float CBTServiceIntervalHelper::GetEngageContextInterval()
{
	return DefaultEngageContextInterval;
}
