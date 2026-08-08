#include "Core/Debug/FDebugOverlayFocusResolver.h"

#include "Character/Enemy/CEnemy.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

#if !UE_BUILD_SHIPPING
namespace
{
	// ===== Actor Lookup Helpers =====

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

	// ===== Focus Lookup =====

	ACEnemy* FindClosestFocusEnemy(UWorld* InWorld, const APawn* InViewerPawn, float& OutDistance)
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

	ACEnemy* ResolveRecentCombatFocusFromSnapshot(UWorld* InWorld, bool& bOutHasRecentCombatEvidence)
	{
		bOutHasRecentCombatEvidence = false;

		FDebugOverlayRecentCombatPair pair;
		if (!FDebugOverlaySnapshotStore::TryGetRecentCombatPair(InWorld, pair))
		{
			return nullptr;
		}

		bOutHasRecentCombatEvidence = true;

		if (ACEnemy* focusEnemy = Cast<ACEnemy>(pair.TargetActor.Get()))
		{
			return focusEnemy;
		}

		if (ACEnemy* sourceEnemy = Cast<ACEnemy>(pair.SourceActor.Get()))
		{
			return sourceEnemy;
		}

		return nullptr;
	}
}
#endif

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveNearestFocus(UWorld* World, const APawn* ViewerPawn, float Radius)
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
	ACEnemy* closestFocusEnemy = FindClosestFocusEnemy(World, ViewerPawn, closestDistance);
	if (!IsValid(closestFocusEnemy))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::NoTarget;
		return result;
	}

	const FString closestFocusName = GetNameSafe(closestFocusEnemy);
	if (closestDistance > Radius)
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::OutOfRange;
		result.ActorName = closestFocusName;
		result.Distance = closestDistance;
		return result;
	}

	result.FocusActor = closestFocusEnemy;
	result.Source = EDebugOverlayFocusSource::NearestFocus;
	result.Outcome = EDebugOverlayFocusResolveOutcome::Selected;
	result.ActorName = closestFocusName;
	result.Distance = closestDistance;
	return result;
#endif
}

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveOutlinerFocus(UWorld* World, const APawn* ViewerPawn, const FString& ActorName)
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

	AActor* focusActor = FindDebugOverlayActorByName(World, trimmedActorName);
	if (!IsValid(focusActor))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::NoActor;
		result.ActorName = trimmedActorName;
		return result;
	}

	const FString focusActorName = GetNameSafe(focusActor);
	const FString focusActorClassName = GetNameSafe(focusActor->GetClass());

	ACEnemy* focusEnemy = Cast<ACEnemy>(focusActor);
	if (!IsValid(focusEnemy))
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::FocusActorIsNotEnemy;
		result.ActorName = focusActorName;
		result.ClassName = focusActorClassName;
		return result;
	}

	result.FocusActor = focusEnemy;
	result.Source = EDebugOverlayFocusSource::OutlinerFocus;
	result.Outcome = EDebugOverlayFocusResolveOutcome::Selected;
	result.ActorName = focusActorName;
	result.ClassName = focusActorClassName;
	return result;
#endif
}

FDebugOverlayFocusResolveResult FDebugOverlayFocusResolver::ResolveRecentCombatFocus(UWorld* World, const APawn* ViewerPawn, float FallbackRadius)
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
	if (ACEnemy* recentEnemy = ResolveRecentCombatFocusFromSnapshot(World, bHasRecentCombatEvidence))
	{
		const FString recentEnemyName = GetNameSafe(recentEnemy);
		const FString recentEnemyClassName = GetNameSafe(recentEnemy->GetClass());
		const float recentEnemyDistance = FVector::Dist(ViewerPawn->GetActorLocation(), recentEnemy->GetActorLocation());

		result.FocusActor = recentEnemy;
		result.Source = EDebugOverlayFocusSource::RecentCombatFocus;
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
	ACEnemy* closestFocusEnemy = FindClosestFocusEnemy(World, ViewerPawn, closestDistance);
	if (!IsValid(closestFocusEnemy))
	{
		if (result.Outcome == EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence)
		{
			return result;
		}

		result.Outcome = EDebugOverlayFocusResolveOutcome::NoTarget;
		return result;
	}

	const FString closestFocusName = GetNameSafe(closestFocusEnemy);
	const FString closestFocusClassName = GetNameSafe(closestFocusEnemy->GetClass());
	if (closestDistance > FallbackRadius)
	{
		result.Outcome = EDebugOverlayFocusResolveOutcome::OutOfRange;
		result.ActorName = closestFocusName;
		result.ClassName = closestFocusClassName;
		result.Distance = closestDistance;
		return result;
	}

	result.FocusActor = closestFocusEnemy;
	result.Source = EDebugOverlayFocusSource::WorldScanFallback;
	result.Outcome = EDebugOverlayFocusResolveOutcome::Selected;
	result.ActorName = closestFocusName;
	result.ClassName = closestFocusClassName;
	result.Distance = closestDistance;
	return result;
#endif
}
