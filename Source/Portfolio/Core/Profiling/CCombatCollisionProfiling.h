#pragma once

#include "CoreMinimal.h"

class FCombatCollisionProfiling
{
public:
	// Gate
	static bool ShouldSkipEnemyHitProcessing(const class AActor* InOwnerActor);
	static bool ShouldSkipEnemyWeaponActorCreation(const class AActor* InOwnerActor);
};
