#include "Core/Debug/FDebugOverlayFocusResolver.h"

#include "Character/Enemy/CEnemy.h"

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
		result.SummaryText = TEXT("NearestFailed | InvalidContext");
		return result;
	}

	float closestDistance = 0.f;
	ACEnemy* closestEnemy = FindClosestDebugOverlayEnemy(World, ViewerPawn, closestDistance);
	if (!IsValid(closestEnemy))
	{
		result.Status = EDebugOverlayFocusResolveStatus::NoEnemy;
		result.SummaryText = FString::Printf(TEXT("NearestFailed | NoEnemy | Radius: %.0f"), Radius);
		return result;
	}

	result.Distance = closestDistance;
	result.ActorName = GetNameSafe(closestEnemy);
	if (closestDistance > Radius)
	{
		result.Status = EDebugOverlayFocusResolveStatus::OutOfRange;
		result.SummaryText = FString::Printf(
			TEXT("NearestFailed | OutOfRange | Closest: %.0f | Radius: %.0f"),
			closestDistance,
			Radius);
		return result;
	}

	result.Status = EDebugOverlayFocusResolveStatus::Selected;
	result.FocusActor = closestEnemy;
	result.FocusSource = EDebugOverlayTargetSource::Nearest;
	result.SummaryText = FString::Printf(
		TEXT("NearestSelected | Target: %s | Distance: %.0f | Radius: %.0f"),
		*result.ActorName,
		closestDistance,
		Radius);
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
		result.SummaryText = TEXT("EditorSelectFailed | InvalidContext");
		return result;
	}

	if (result.ActorName.IsEmpty())
	{
		result.Status = EDebugOverlayFocusResolveStatus::NoActorName;
		result.SummaryText = TEXT("EditorSelectFailed | NoActorName");
		return result;
	}

	AActor* targetActor = FindDebugOverlayActorByName(World, result.ActorName);
	if (!IsValid(targetActor))
	{
		result.Status = EDebugOverlayFocusResolveStatus::NoActor;
		result.SummaryText = FString::Printf(TEXT("EditorSelectFailed | NoActor | Name: %s"), *result.ActorName);
		return result;
	}

	result.ActorName = GetNameSafe(targetActor);
	result.ClassName = GetNameSafe(targetActor->GetClass());

	ACEnemy* targetEnemy = Cast<ACEnemy>(targetActor);
	if (!IsValid(targetEnemy))
	{
		result.Status = EDebugOverlayFocusResolveStatus::NotEnemy;
		result.SummaryText = FString::Printf(TEXT("EditorSelectFailed | NotEnemy | Target: %s"), *result.ActorName);
		return result;
	}

	result.Status = EDebugOverlayFocusResolveStatus::Selected;
	result.FocusActor = targetEnemy;
	result.FocusSource = EDebugOverlayTargetSource::EditorSelection;
	result.SummaryText = FString::Printf(TEXT("EditorSelected | Target: %s"), *GetNameSafe(targetEnemy));
	return result;
#endif
}
