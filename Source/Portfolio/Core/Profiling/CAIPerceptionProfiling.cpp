#include "Core/Profiling/CAIPerceptionProfiling.h"

#include "Character/Enemy/CEnemy.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarDisableEnemyPerception(
		TEXT("Portfolio.AI.RuntimeLOD.DisableEnemyPerception"),
		0,
		TEXT("Disable ACEnemy AI perception for runtime LOD measurement. 0: enable perception, 1: disable ACEnemy perception."),
		ECVF_Default);
#endif
}

// Gate

bool FAIPerceptionProfiling::ShouldDisableEnemyPerception(const AActor* InOwnerActor)
{
#if !UE_BUILD_SHIPPING
	if (CVarDisableEnemyPerception.GetValueOnGameThread() == 0) return false;

	return IsValid(InOwnerActor) && InOwnerActor->IsA<ACEnemy>();
#else
	return false;
#endif
}
