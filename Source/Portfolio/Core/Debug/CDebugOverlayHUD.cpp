#include "Core/Debug/CDebugOverlayHUD.h"

#include "Character/Enemy/CEnemy.h"
#include "Core/Debug/CDebugOverlayTargetComponent.h"
#include "Core/Debug/FDebugOverlayCanvasRenderer.h"
#include "Core/Debug/FDebugOverlaySnapshotStore.h"
#include "Core/Debug/FDebugOverlayTextFormatter.h"
#include "Core/Debug/FDebugOverlayViewDataBuilder.h"

#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#if !UE_BUILD_SHIPPING
namespace
{
	static constexpr float DebugOverlayEnemyScanCooldownSeconds = 0.5f;
	static constexpr float DebugOverlayRecentCombatTargetStaleSeconds = 3.0f;

	FString FormatAgeSeconds(float InAgeSeconds)
	{
		return FString::Printf(TEXT("%.2f"), FMath::Max(0.f, InAgeSeconds));
	}

	void AppendOverlayLine(TArray<FString>& InOutLines, const FString& InLine)
	{
		InOutLines.Add(InLine);
	}

	void AppendTargetSelectionSummary(TArray<FString>& InOutLines, const UCDebugOverlayTargetComponent* InTargetComp)
	{
		if (!IsValid(InTargetComp)) return;
		if (!InTargetComp->HasDebugOverlaySelectionSummary()) return;

		AppendOverlayLine(InOutLines, FString::Printf(TEXT("EnemySelect: %s"), *InTargetComp->GetDebugOverlaySelectionSummary()));
	}
}
#endif

#if !UE_BUILD_SHIPPING
ACEnemy* ACDebugOverlayHUD::ResolveDisplayEnemy(TArray<FString>& OutSourceLines)
{
	if (ACEnemy* targetComponentEnemy = ResolveTargetComponentEnemy(OutSourceLines))
	{
		return targetComponentEnemy;
	}

	AppendOverlayLine(OutSourceLines, TEXT("EnemySource: None"));
	if (const APlayerController* owningPlayerController = GetOwningPlayerController())
	{
		const UCDebugOverlayTargetComponent* targetComp = owningPlayerController->FindComponentByClass<UCDebugOverlayTargetComponent>();
		AppendTargetSelectionSummary(OutSourceLines, targetComp);
	}

	return nullptr;
}

ACEnemy* ACDebugOverlayHUD::ResolveTargetComponentEnemy(TArray<FString>& OutSourceLines) const
{
	const APlayerController* owningPlayerController = GetOwningPlayerController();
	if (!IsValid(owningPlayerController)) return nullptr;

	const UCDebugOverlayTargetComponent* targetComp = owningPlayerController->FindComponentByClass<UCDebugOverlayTargetComponent>();
	if (!IsValid(targetComp)) return nullptr;

	ACEnemy* targetEnemy = Cast<ACEnemy>(targetComp->GetDebugOverlayTargetActor());
	if (!IsValid(targetEnemy)) return nullptr;

	AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemySource: %s"), *targetComp->GetDebugOverlayTargetSource()));
	AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemyTarget: %s"), *targetComp->GetDebugOverlayTargetSummary()));
	AppendTargetSelectionSummary(OutSourceLines, targetComp);
	return targetEnemy;
}

ACEnemy* ACDebugOverlayHUD::ResolveRecentCombatEnemy(const APawn* InViewerPawn, TArray<FString>& OutSourceLines) const
{
	FDebugOverlayRecentCombatPair recentCombatPair;
	if (!FDebugOverlaySnapshotStore::TryGetRecentCombatPair(GetWorld(), recentCombatPair))
	{
		return nullptr;
	}

	const UWorld* world = GetWorld();
	const float currentTime = IsValid(world) ? world->GetTimeSeconds() : 0.f;
	const float pairAge = currentTime - recentCombatPair.WorldTimeSeconds;
	const bool bPairStale = pairAge > DebugOverlayRecentCombatTargetStaleSeconds;

	AActor* sourceActor = recentCombatPair.SourceActor.Get();
	AActor* targetActor = recentCombatPair.TargetActor.Get();
	const bool bSourceInvalid = !IsValid(sourceActor);
	const bool bTargetInvalid = !IsValid(targetActor);
	bool bRecentCombatPairMatched = false;

	if (!bPairStale && !bSourceInvalid && !bTargetInvalid && IsValid(InViewerPawn))
	{
		ACEnemy* recentEnemy = nullptr;
		if (sourceActor == InViewerPawn)
		{
			recentEnemy = Cast<ACEnemy>(targetActor);
			bRecentCombatPairMatched = true;
		}
		else if (targetActor == InViewerPawn)
		{
			recentEnemy = Cast<ACEnemy>(sourceActor);
			bRecentCombatPairMatched = true;
		}

		if (IsValid(recentEnemy))
		{
			AppendOverlayLine(OutSourceLines, TEXT("EnemySource: RecentCombatTarget"));
			AppendOverlayLine(OutSourceLines, FString::Printf(
				TEXT("EnemyRecentCombat: Source: %s | Target: %s | Age: %s"),
				*recentCombatPair.SourceName,
				*recentCombatPair.TargetName,
				*FormatAgeSeconds(pairAge)));
			return recentEnemy;
		}
	}

	if (bPairStale || bSourceInvalid || bTargetInvalid)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(
			TEXT("EnemyRecentCombat: Stale | Source: %s | Target: %s | Age: %s"),
			*recentCombatPair.SourceName,
			*recentCombatPair.TargetName,
			*FormatAgeSeconds(pairAge)));
	}
	else if (!bRecentCombatPairMatched)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(
			TEXT("EnemyRecentCombat: NotMatched | Source: %s | Target: %s | Age: %s"),
			*recentCombatPair.SourceName,
			*recentCombatPair.TargetName,
			*FormatAgeSeconds(pairAge)));
	}

	return nullptr;
}

ACEnemy* ACDebugOverlayHUD::ResolveWorldScanFallbackEnemy(TArray<FString>& OutSourceLines)
{
	RefreshCachedEnemyIfNeeded();
	if (LastEnemyScanCount == 0)
	{
		AppendOverlayLine(OutSourceLines, TEXT("EnemySource: None"));
		return nullptr;
	}

	if (LastEnemyScanCount > 1)
	{
		AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemySource: Ambiguous(Count=%d)"), LastEnemyScanCount));
		return nullptr;
	}

	ACEnemy* fallbackEnemy = CachedEnemy.Get();
	if (!IsValid(fallbackEnemy))
	{
		AppendOverlayLine(OutSourceLines, TEXT("EnemySource: Stale"));
		return nullptr;
	}

	AppendOverlayLine(OutSourceLines, TEXT("EnemySource: WorldScanFallback"));
	AppendOverlayLine(OutSourceLines, FString::Printf(TEXT("EnemyFallback: Selected: %s | Policy: FirstValid | Count: 1"), *GetNameSafe(fallbackEnemy)));
	return fallbackEnemy;
}

void ACDebugOverlayHUD::RefreshCachedEnemyIfNeeded()
{
	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	const float currentTime = world->GetTimeSeconds();
	if (CachedEnemy.IsValid() && currentTime - LastEnemyScanTimeSeconds < DebugOverlayEnemyScanCooldownSeconds) return;
	if (!CachedEnemy.IsValid() && LastEnemyScanTimeSeconds >= 0.f && currentTime - LastEnemyScanTimeSeconds < DebugOverlayEnemyScanCooldownSeconds) return;

	LastEnemyScanTimeSeconds = currentTime;
	LastEnemyScanCount = 0;
	CachedEnemy.Reset();

	for (TActorIterator<ACEnemy> actorIt(world); actorIt; ++actorIt)
	{
		ACEnemy* enemy = *actorIt;
		if (!IsValid(enemy)) continue;

		++LastEnemyScanCount;
		if (!CachedEnemy.IsValid())
		{
			CachedEnemy = enemy;
		}
	}
}
#endif

void ACDebugOverlayHUD::DrawHUD()
{
#if !UE_BUILD_SHIPPING
	Super::DrawHUD();

	if (!FDebugOverlaySnapshotStore::IsEnabled()) return;

	UWorld* world = GetWorld();
	TArray<FString> enemySourceLines;
	const ACEnemy* enemy = ResolveDisplayEnemy(enemySourceLines);

	FDebugOverlayViewDataBuildContext context;
	context.WorldContextObject = world;
	context.World = world;
	context.ViewerPawn = GetOwningPawn();
	context.DisplayEnemy = enemy;
	context.EnemySourceLines = enemySourceLines;

	const FDebugOverlayViewData viewData = FDebugOverlayViewDataBuilder::Build(context);
	const FDebugOverlayTextPanels textPanels = FDebugOverlayTextFormatter::Format(viewData);
	FDebugOverlayCanvasRenderer::Draw(*this, Canvas, textPanels);
#endif
}
