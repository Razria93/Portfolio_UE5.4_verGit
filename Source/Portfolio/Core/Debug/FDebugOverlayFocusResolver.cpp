#include "Core/Debug/FDebugOverlayFocusResolver.h"

#include "Character/Enemy/CEnemy.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

#if !UE_BUILD_SHIPPING
namespace
{
	AActor* FindDebugOverlayActorByName(UWorld* InWorld, const FString& InActorName)
	{
		if (!IsValid(InWorld) || InActorName.IsEmpty()) return nullptr;

		for (TActorIterator<AActor> actorIt(InWorld); actorIt; ++actorIt)
		{
			AActor* actor = *actorIt;
			if (!IsValid(actor)) continue;

			if (actor->GetName().Equals(InActorName, ESearchCase::IgnoreCase))
			{
				return actor;
			}

#if WITH_EDITOR
			if (actor->GetActorLabel().Equals(InActorName, ESearchCase::IgnoreCase))
			{
				return actor;
			}
#endif
		}

		return nullptr;
	}

	ACEnemy* FindClosestFocusTarget(UWorld* InWorld, const APawn* InViewerPawn, float& OutDistance)
	{
		OutDistance = 0.f;
		if (!IsValid(InWorld) || !IsValid(InViewerPawn)) return nullptr;

		const FVector origin = InViewerPawn->GetActorLocation();

		ACEnemy* closestTarget = nullptr;
		float closestDistanceSquared = TNumericLimits<float>::Max();

		for (TActorIterator<ACEnemy> enemyIt(InWorld); enemyIt; ++enemyIt)
		{
			ACEnemy* enemy = *enemyIt;
			if (!IsValid(enemy)) continue;

			const float distanceSquared = FVector::DistSquared(origin, enemy->GetActorLocation());
			if (distanceSquared > closestDistanceSquared) continue;

			closestDistanceSquared = distanceSquared;
			closestTarget = enemy;
		}

		if (!IsValid(closestTarget)) return nullptr;

		OutDistance = FMath::Sqrt(closestDistanceSquared);
		return closestTarget;
	}

	ACEnemy* ResolveRecentCombatTargetFromSnapshot(UWorld* InWorld, bool& bOutHasRecentCombatEvidence)
	{
		bOutHasRecentCombatEvidence = false;

		FDebugOverlayRecentCombatPair pair;
		if (!FDebugOverlaySnapshotStore::TryGetRecentCombatPair(InWorld, pair))
		{
			return nullptr;
		}

		bOutHasRecentCombatEvidence = true;

		if (ACEnemy* targetEnemy = Cast<ACEnemy>(pair.TargetActor.Get()))
		{
			return targetEnemy;
		}

		if (ACEnemy* sourceEnemy = Cast<ACEnemy>(pair.SourceActor.Get()))
		{
			return sourceEnemy;
		}

		return nullptr;
	}
}
#endif

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveNearestTarget(UWorld* World, const APawn* ViewerPawn, float Radius)
{
	FDebugOverlayFocusResolveResult result;
	result.Radius = Radius;

#if UE_BUILD_SHIPPING
	return result;
#else
	if (!IsValid(World) || !IsValid(ViewerPawn))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::InvalidContext;
		return result;
	}

	float closestDistance = 0.f;
	ACEnemy* closestTarget = FindClosestFocusTarget(World, ViewerPawn, closestDistance);
	if (!IsValid(closestTarget))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::NoTarget;
		return result;
	}

	const FString closestTargetName = GetNameSafe(closestTarget);
	if (closestDistance > Radius)
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::OutOfRange;
		result.ActorName = closestTargetName;
		result.Distance = closestDistance;
		return result;
	}

	result.FocusActor = closestTarget;
	result.Source = EDebugOverlayFocusSource::NearestTarget;
	result.Outcome = EDebugOverlayFocusResolveOutcome::Selected;
	result.ActorName = closestTargetName;
	result.Distance = closestDistance;
	return result;
#endif
}

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveOutlinerTarget(UWorld* World, const APawn* ViewerPawn, const FString& ActorName)
{
	FDebugOverlayFocusResolveResult result;
	const FString trimmedActorName = ActorName.TrimStartAndEnd();

#if UE_BUILD_SHIPPING
	return result;
#else
	if (!IsValid(World) || !IsValid(ViewerPawn))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::InvalidContext;
		return result;
	}

	if (trimmedActorName.IsEmpty())
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::NoActorName;
		result.ActorName = trimmedActorName;
		return result;
	}

	AActor* targetActor = FindDebugOverlayActorByName(World, trimmedActorName);
	if (!IsValid(targetActor))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::NoActor;
		result.ActorName = trimmedActorName;
		return result;
	}

	const FString targetActorName = GetNameSafe(targetActor);
	const FString targetActorClassName = GetNameSafe(targetActor->GetClass());

	ACEnemy* targetEnemy = Cast<ACEnemy>(targetActor);
	if (!IsValid(targetEnemy))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::TargetIsNotEnemy;
		result.ActorName = targetActorName;
		result.ClassName = targetActorClassName;
		return result;
	}

	result.FocusActor = targetEnemy;
	result.Source = EDebugOverlayFocusSource::OutlinerTarget;
	result.Outcome = EDebugOverlayFocusResolveOutcome::Selected;
	result.ActorName = targetActorName;
	result.ClassName = targetActorClassName;
	return result;
#endif
}

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveRecentCombatTarget(UWorld* World, const APawn* ViewerPawn, float FallbackRadius)
{
	FDebugOverlayFocusResolveResult result;
	result.Radius = FallbackRadius;

#if UE_BUILD_SHIPPING
	return result;
#else
	if (!IsValid(World) || !IsValid(ViewerPawn))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::InvalidContext;
		return result;
	}

	bool bHasRecentCombatEvidence = false;
	if (ACEnemy* recentEnemy = ResolveRecentCombatTargetFromSnapshot(World, bHasRecentCombatEvidence))
	{
		const FString recentEnemyName = GetNameSafe(recentEnemy);
		const FString recentEnemyClassName = GetNameSafe(recentEnemy->GetClass());
		const float recentEnemyDistance = FVector::Dist(ViewerPawn->GetActorLocation(), recentEnemy->GetActorLocation());

		result.FocusActor = recentEnemy;
		result.Source = EDebugOverlayFocusSource::RecentCombat;
		result.Outcome = EDebugOverlayFocusResolveOutcome::Selected;
		result.ActorName = recentEnemyName;
		result.ClassName = recentEnemyClassName;
		result.Distance = recentEnemyDistance;
		return result;
	}

	if (!bHasRecentCombatEvidence)
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence;
	}

	float closestDistance = 0.f;
	ACEnemy* closestTarget = FindClosestFocusTarget(World, ViewerPawn, closestDistance);
	if (!IsValid(closestTarget))
	{
		if (result.Outcome == EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence)
		{
			return result;
		}

		result.Outcome = EDebugOverlayFocusResolveOutcome::NoTarget;
		return result;
	}

	const FString closestTargetName = GetNameSafe(closestTarget);
	const FString closestTargetClassName = GetNameSafe(closestTarget->GetClass());
	if (closestDistance > FallbackRadius)
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::OutOfRange;
		result.ActorName = closestTargetName;
		result.ClassName = closestTargetClassName;
		result.Distance = closestDistance;
		return result;
	}

	result.FocusActor = closestTarget;
	result.Source = EDebugOverlayFocusSource::WorldScanFallback;
	result.Outcome = EDebugOverlayFocusResolveOutcome::Selected;
	result.ActorName = closestTargetName;
	result.ClassName = closestTargetClassName;
	result.Distance = closestDistance;
	return result;
#endif
}
