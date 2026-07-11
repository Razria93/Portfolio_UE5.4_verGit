#include "AI/RuntimeLOD/CAIMovementRuntimeLODPolicy.h"

#include "Character/Enemy/CEnemy.h"
#include "HAL/IConsoleManager.h"

namespace
{
	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyMovementMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyMovementMode"),
		0,
		TEXT("Controls ACEnemy movement runtime LOD mode. 0: default, 1: disable movement state refresh, 2: block movement intent."),
		ECVF_Default);
}

int32 FAIMovementRuntimeLODPolicy::GetEnemyMovementMode()
{
	return FMath::Clamp(CVarAIRuntimeLODEnemyMovementMode.GetValueOnGameThread(), 0, 2);
}

bool FAIMovementRuntimeLODPolicy::IsEnemyMovementRuntimeLODTarget(const AActor* InOwner)
{
	return IsValid(InOwner) && InOwner->IsA<ACEnemy>();
}

bool FAIMovementRuntimeLODPolicy::ShouldDisableMovementStateRefresh(int32 InMovementMode)
{
	return InMovementMode == 1;
}

bool FAIMovementRuntimeLODPolicy::ShouldBlockMovementIntent(int32 InMovementMode)
{
	return InMovementMode == 2;
}
