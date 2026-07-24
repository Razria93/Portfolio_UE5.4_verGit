#include "Core/Profiling/CCombatCollisionProfiling.h"

#include "Character/Enemy/CEnemy.h"

#include "HAL/IConsoleManager.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarDisableEnemyHitProcessing(
		TEXT("Portfolio.AI.RuntimeLOD.DisableEnemyHitProcessing"),
		0,
		TEXT("Disable ACEnemy hit processing for combat collision profiling. 0: process hit, 1: skip ACEnemy hit processing after overlap."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarDisableEnemyWeaponActor(
		TEXT("Portfolio.AI.RuntimeLOD.DisableEnemyWeaponActor"),
		0,
		TEXT("Disable ACEnemy WeaponActor creation for runtime LOD measurement. 0: spawn WeaponActor, 1: skip ACEnemy WeaponActor."),
		ECVF_Default);
#endif

	bool IsEnemyProfilingTarget(const AActor* InOwnerActor)
	{
		return IsValid(InOwnerActor) && InOwnerActor->IsA<ACEnemy>();
	}
}

bool FCombatCollisionProfiling::ShouldSkipEnemyHitProcessing(const AActor* InOwnerActor)
{
#if !UE_BUILD_SHIPPING
	if (CVarDisableEnemyHitProcessing.GetValueOnGameThread() == 0) return false;

	return IsEnemyProfilingTarget(InOwnerActor);
#else
	return false;
#endif
}

bool FCombatCollisionProfiling::ShouldSkipEnemyWeaponActorCreation(const AActor* InOwnerActor)
{
#if !UE_BUILD_SHIPPING
	if (CVarDisableEnemyWeaponActor.GetValueOnGameThread() == 0) return false;

	return IsEnemyProfilingTarget(InOwnerActor);
#else
	return false;
#endif
}
