#pragma once

#include "CoreMinimal.h"

class AActor;

class FAIAnimationRuntimeLODPolicy
{
public:
	static int32 GetEnemyAnimationMode();
	static int32 GetEnemyAnimationMode(const AActor* InOwner);
	static bool IsEnemyAnimationRuntimeLODTarget(const AActor* InOwner);

	static float GetReducedAnimationRefreshInterval();
	static bool ShouldAuditAnimationRefresh();

private:
	static int32 GetStateBasedAnimationMode(const AActor* InOwner);
};
