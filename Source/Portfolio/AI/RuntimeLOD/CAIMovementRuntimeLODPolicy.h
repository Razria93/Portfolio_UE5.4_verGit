#pragma once

#include "CoreMinimal.h"

class AActor;

class FAIMovementRuntimeLODPolicy
{
public:
	static int32 GetEnemyMovementMode();
	static bool IsEnemyMovementRuntimeLODTarget(const AActor* InOwner);

	static bool ShouldDisableMovementStateRefresh(int32 InMovementMode);
	static bool ShouldBlockMovementIntent(int32 InMovementMode);
};
