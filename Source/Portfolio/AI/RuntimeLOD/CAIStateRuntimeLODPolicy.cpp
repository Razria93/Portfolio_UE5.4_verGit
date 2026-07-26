#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"

#include "HAL/IConsoleManager.h"

namespace
{
	enum class EAIStateRuntimeLODPolicyMode : int32
	{
		PerSystem = 0,
		StateBased = 1,
	};

	constexpr int32 ToStateRuntimeLODPolicyModeValue(EAIStateRuntimeLODPolicyMode InMode)
	{
		return static_cast<int32>(InMode);
	}

	TAutoConsoleVariable<int32> CVarAIStateRuntimeLODPolicyMode(
		TEXT("Portfolio.AI.RuntimeLOD.StatePolicyMode"),
		ToStateRuntimeLODPolicyModeValue(EAIStateRuntimeLODPolicyMode::PerSystem),
		TEXT("Controls AI Runtime LOD policy source. 0: per-system Runtime LOD CVar modes, 1: state-based Runtime LOD tier snapshot."),
		ECVF_Default);
}

int32 FAIStateRuntimeLODPolicy::GetStatePolicyMode()
{
	return FMath::Clamp(
		CVarAIStateRuntimeLODPolicyMode.GetValueOnGameThread(),
		ToStateRuntimeLODPolicyModeValue(EAIStateRuntimeLODPolicyMode::PerSystem),
		ToStateRuntimeLODPolicyModeValue(EAIStateRuntimeLODPolicyMode::StateBased));
}

bool FAIStateRuntimeLODPolicy::ShouldUseStateBasedPolicy()
{
	return GetStatePolicyMode() == ToStateRuntimeLODPolicyModeValue(EAIStateRuntimeLODPolicyMode::StateBased);
}
