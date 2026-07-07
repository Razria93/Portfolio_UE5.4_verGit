#include "AI/BehaviorTree/Service/CBTServiceIntervalHelper.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarBTUpdateIntervalMode(
		TEXT("Portfolio.AI.RuntimeLOD.BTUpdateIntervalMode"),
		0,
		TEXT("Controls AI BT service update interval profiling mode. 0: default, 1: reduced, 2: aggressive reduced."),
		ECVF_Default);

	constexpr float DefaultAIContextInterval = 0.1f;
	constexpr float DefaultAIIntentStateInterval = 0.2f;
	constexpr float DefaultEngageContextInterval = 0.1f;

	constexpr float ReducedAIContextInterval = 0.2f;
	constexpr float ReducedAIIntentStateInterval = 0.3f;
	constexpr float ReducedEngageContextInterval = 0.2f;

	constexpr float AggressiveAIContextInterval = 0.4f;
	constexpr float AggressiveAIIntentStateInterval = 0.5f;
	constexpr float AggressiveEngageContextInterval = 0.3f;

	int32 GetBTUpdateIntervalMode()
	{
		return FMath::Clamp(CVarBTUpdateIntervalMode.GetValueOnGameThread(), 0, 2);
	}
}

float CBTServiceIntervalHelper::GetAIContextInterval()
{
	switch (GetBTUpdateIntervalMode())
	{
	case 1:
		return ReducedAIContextInterval;

	case 2:
		return AggressiveAIContextInterval;

	case 0:
	default:
		return DefaultAIContextInterval;
	}
}

float CBTServiceIntervalHelper::GetAIIntentStateInterval()
{
	switch (GetBTUpdateIntervalMode())
	{
	case 1:
		return ReducedAIIntentStateInterval;

	case 2:
		return AggressiveAIIntentStateInterval;

	case 0:
	default:
		return DefaultAIIntentStateInterval;
	}
}

float CBTServiceIntervalHelper::GetEngageContextInterval()
{
	switch (GetBTUpdateIntervalMode())
	{
	case 1:
		return ReducedEngageContextInterval;

	case 2:
		return AggressiveEngageContextInterval;

	case 0:
	default:
		return DefaultEngageContextInterval;
	}
}
