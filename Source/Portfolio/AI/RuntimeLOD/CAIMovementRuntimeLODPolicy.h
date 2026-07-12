#pragma once

#include "CoreMinimal.h"

class AActor;

class FAIMovementRuntimeLODPolicy
{
public:
	static int32 GetEnemyMovementMode();
	static int32 GetEnemyMovementMode(const AActor* InOwner);
	static bool IsEnemyMovementRuntimeLODTarget(const AActor* InOwner);

	static bool ShouldDisableMovementStateRefresh(int32 InMovementMode);
	static bool ShouldBlockMovementIntent(int32 InMovementMode);

private:
	static int32 GetStateBasedMovementMode(const AActor* InOwner);
};
