#include "AI/RuntimeLOD/CAIStateRuntimeLODPolicy.h"

#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarAIStateRuntimeLODPolicyMode(
		TEXT("Portfolio.AI.RuntimeLOD.StatePolicyMode"),
		0,
		TEXT("Controls AI Runtime LOD policy source. 0: per-system RuntimeLOD CVar modes, 1: state-based RuntimeLOD tier snapshot."),
		ECVF_Default);
}

int32 FAIStateRuntimeLODPolicy::GetStatePolicyMode()
{
	return FMath::Clamp(CVarAIStateRuntimeLODPolicyMode.GetValueOnGameThread(), 0, 1);
}

bool FAIStateRuntimeLODPolicy::ShouldUseStateBasedPolicy()
{
	return GetStatePolicyMode() > 0;
}
