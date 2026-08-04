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

	ACEnemy* FindClosestDebugOverlayEnemy(UWorld* InWorld, const APawn* InViewerPawn, float& OutDistance)
	{
		OutDistance = 0.f;
		if (!IsValid(InWorld) || !IsValid(InViewerPawn)) return nullptr;

		const FVector origin = InViewerPawn->GetActorLocation();

		ACEnemy* closestEnemy = nullptr;
		float closestDistanceSquared = TNumericLimits<float>::Max();

		for (TActorIterator<ACEnemy> enemyIt(InWorld); enemyIt; ++enemyIt)
		{
			ACEnemy* enemy = *enemyIt;
			if (!IsValid(enemy)) continue;

			const float distanceSquared = FVector::DistSquared(origin, enemy->GetActorLocation());
			if (distanceSquared > closestDistanceSquared) continue;

			closestDistanceSquared = distanceSquared;
			closestEnemy = enemy;
		}

		if (!IsValid(closestEnemy)) return nullptr;

		OutDistance = FMath::Sqrt(closestDistanceSquared);
		return closestEnemy;
	}

	ACEnemy* ResolveRecentCombatEnemyActor(UWorld* InWorld, ACEnemy*& OutSourceEnemy)
	{
		OutSourceEnemy = nullptr;

		FDebugOverlayRecentCombatPair pair;
		if (!FDebugOverlaySnapshotStore::TryGetRecentCombatPair(InWorld, pair))
		{
			return nullptr;
		}

		if (ACEnemy* targetEnemy = Cast<ACEnemy>(pair.TargetActor.Get()))
		{
			OutSourceEnemy = Cast<ACEnemy>(pair.SourceActor.Get());
			return targetEnemy;
		}

		if (ACEnemy* sourceEnemy = Cast<ACEnemy>(pair.SourceActor.Get()))
		{
			OutSourceEnemy = sourceEnemy;
			return sourceEnemy;
		}

		return nullptr;
	}
}
#endif

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveNearestEnemy(
	UWorld* World,
	const APawn* ViewerPawn,
	float Radius)
{
	FDebugOverlayFocusResolveResult result;
	result.Radius = Radius;

#if UE_BUILD_SHIPPING
	return result;
#else
	if (!IsValid(World) || !IsValid(ViewerPawn))
	{
		result.Status = EDebugOverlayFocusResolveStatus::InvalidContext;
		return result;
	}

	float closestDistance = 0.f;
	ACEnemy* closestEnemy = FindClosestDebugOverlayEnemy(World, ViewerPawn, closestDistance);
	if (!IsValid(closestEnemy))
	{
		result.Status = EDebugOverlayFocusResolveStatus::NoEnemy;
		return result;
	}

	result.Distance = closestDistance;
	result.ActorName = GetNameSafe(closestEnemy);
	if (closestDistance > Radius)
	{
		result.Status = EDebugOverlayFocusResolveStatus::OutOfRange;
		return result;
	}

	result.Status = EDebugOverlayFocusResolveStatus::Selected;
	result.FocusActor = closestEnemy;
	result.FocusSource = EDebugOverlayFocusSource::NearestEnemy;
	return result;
#endif
}

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveActorEnemy(
	UWorld* World,
	const APawn* ViewerPawn,
	const FString& ActorName)
{
	FDebugOverlayFocusResolveResult result;
	result.ActorName = ActorName.TrimStartAndEnd();

#if UE_BUILD_SHIPPING
	return result;
#else
	if (!IsValid(World) || !IsValid(ViewerPawn))
	{
		result.Status = EDebugOverlayFocusResolveStatus::InvalidContext;
		return result;
	}

	if (result.ActorName.IsEmpty())
	{
		result.Status = EDebugOverlayFocusResolveStatus::NoActorName;
		return result;
	}

	AActor* targetActor = FindDebugOverlayActorByName(World, result.ActorName);
	if (!IsValid(targetActor))
	{
		result.Status = EDebugOverlayFocusResolveStatus::NoActor;
		return result;
	}

	result.ActorName = GetNameSafe(targetActor);
	result.ClassName = GetNameSafe(targetActor->GetClass());

	ACEnemy* targetEnemy = Cast<ACEnemy>(targetActor);
	if (!IsValid(targetEnemy))
	{
		result.Status = EDebugOverlayFocusResolveStatus::NotEnemy;
		return result;
	}

	result.Status = EDebugOverlayFocusResolveStatus::Selected;
	result.FocusActor = targetEnemy;
	result.FocusSource = EDebugOverlayFocusSource::EditorSelection;
	return result;
#endif
}

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveRecentCombatEnemy(
	UWorld* World,
	const APawn* ViewerPawn,
	float FallbackRadius)
{
	FDebugOverlayFocusResolveResult result;
	result.Radius = FallbackRadius;

#if UE_BUILD_SHIPPING
	return result;
#else
	if (!IsValid(World) || !IsValid(ViewerPawn))
	{
		result.Status = EDebugOverlayFocusResolveStatus::InvalidContext;
		return result;
	}

	ACEnemy* sourceEnemy = nullptr;
	if (ACEnemy* recentEnemy = ResolveRecentCombatEnemyActor(World, sourceEnemy))
	{
		result.Status = EDebugOverlayFocusResolveStatus::Selected;
		result.FocusActor = recentEnemy;
		result.FocusSource = EDebugOverlayFocusSource::RecentCombat;
		result.ActorName = GetNameSafe(recentEnemy);
		result.ClassName = GetNameSafe(recentEnemy->GetClass());
		result.Distance = FVector::Dist(ViewerPawn->GetActorLocation(), recentEnemy->GetActorLocation());
		return result;
	}

	FDebugOverlayRecentCombatPair pair;
	const bool bHasRecentCombatPair = FDebugOverlaySnapshotStore::TryGetRecentCombatPair(World, pair);
	if (!bHasRecentCombatPair)
	{
		result.Status = EDebugOverlayFocusResolveStatus::NoRecentCombat;
	}

	float closestDistance = 0.f;
	ACEnemy* closestEnemy = FindClosestDebugOverlayEnemy(World, ViewerPawn, closestDistance);
	if (!IsValid(closestEnemy))
	{
		if (result.Status == EDebugOverlayFocusResolveStatus::NoRecentCombat)
		{
			return result;
		}

		result.Status = EDebugOverlayFocusResolveStatus::NoEnemy;
		return result;
	}

	result.Distance = closestDistance;
	result.ActorName = GetNameSafe(closestEnemy);
	result.ClassName = GetNameSafe(closestEnemy->GetClass());
	if (closestDistance > FallbackRadius)
	{
		result.Status = EDebugOverlayFocusResolveStatus::OutOfRange;
		return result;
	}

	result.Status = EDebugOverlayFocusResolveStatus::Selected;
	result.FocusActor = closestEnemy;
	result.FocusSource = EDebugOverlayFocusSource::WorldScanFallback;
	return result;
#endif
}
