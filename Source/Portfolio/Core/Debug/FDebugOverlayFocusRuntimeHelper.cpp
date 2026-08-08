#include "Core/Debug/FDebugOverlayFocusRuntimeHelper.h"

#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Core/Debug/FDebugOverlayFocusLogHelper.h"
#include "Core/Debug/FDebugOverlayFocusResolver.h"
#include "HAL/IConsoleManager.h"

namespace
{
	// ===== CVars =====

	static TAutoConsoleVariable<float> CVarDebugOverlayNearestFocusRadius(
		TEXT("Portfolio.DebugOverlay.NearestFocusRadius"),
		3000.f,
		TEXT("Nearest focus search radius used by Debug Overlay focus commands."),
		ECVF_Default);

	// ===== Recent Combat Driver Policy =====

	bool IsRecentCombatOutcomeLive(EDebugOverlayFocusResolveOutcome InOutcome)
	{
		switch (InOutcome)
		{
		case EDebugOverlayFocusResolveOutcome::Selected:
		case EDebugOverlayFocusResolveOutcome::NoTarget:
		case EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence:
		case EDebugOverlayFocusResolveOutcome::OutOfRange:
			return true;
		default:
			return false;
		}
	}

	EDebugOverlayRecentFocusState BuildRecentFocusStateFromOutcome(EDebugOverlayFocusResolveOutcome InOutcome)
	{
		switch (InOutcome)
		{
		case EDebugOverlayFocusResolveOutcome::Selected:
			return EDebugOverlayRecentFocusState::Selected;
		case EDebugOverlayFocusResolveOutcome::NoTarget:
			return EDebugOverlayRecentFocusState::NoFocusFound;
		case EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence:
			return EDebugOverlayRecentFocusState::NoRecentCombatEvidence;
		case EDebugOverlayFocusResolveOutcome::OutOfRange:
			return EDebugOverlayRecentFocusState::ClosestOutOfRange;
		default:
			return EDebugOverlayRecentFocusState::None;
		}
	}

	void UpdateRecentFocusState(UCDebugOverlayFocusComponent* InFocusComponent, EDebugOverlayRecentFocusState InRecentFocusState)
	{
		if (!IsValid(InFocusComponent)) return;

		if (InRecentFocusState != EDebugOverlayRecentFocusState::None)
		{
			InFocusComponent->SetDebugOverlayRecentFocusState(InRecentFocusState);
			return;
		}

		InFocusComponent->ClearDebugOverlayRecentFocusState();
	}

	void ApplyRecentCombatOutcomePolicy(UCDebugOverlayFocusComponent* InFocusComponent, EDebugOverlayFocusResolveOutcome InOutcome)
	{
		if (!IsValid(InFocusComponent)) return;

		if (IsRecentCombatOutcomeLive(InOutcome))
		{
			const EDebugOverlayRecentFocusState recentFocusState = BuildRecentFocusStateFromOutcome(InOutcome);
			UpdateRecentFocusState(InFocusComponent, recentFocusState);
			return;
		}

		InFocusComponent->ClearDebugOverlayFocusDriver();
		InFocusComponent->ClearDebugOverlayRecentFocusState();
	}

	// ===== Focus Result Apply =====

	void ApplyDebugOverlayFocusResolveResult(UCDebugOverlayFocusComponent* InFocusComponent, const FDebugOverlayFocusResolveResult& InResult, EDebugOverlayFocusDriver InDriverOnSuccess, bool bUpdateDriver, bool bKeepFocusOnFailure)
	{
		if (!IsValid(InFocusComponent)) return;

		if (InResult.Outcome == EDebugOverlayFocusResolveOutcome::Selected)
		{
			InFocusComponent->SetDebugOverlayFocusActorAndSource(InResult.FocusActor.Get(), InResult.Source);
			if (bUpdateDriver)
			{
				InFocusComponent->SetDebugOverlayFocusDriver(InDriverOnSuccess);
			}
			return;
		}

		if (bKeepFocusOnFailure)
		{
			return;
		}

		InFocusComponent->ClearDebugOverlayFocusActorAndSource();
		if (bUpdateDriver)
		{
			InFocusComponent->ClearDebugOverlayFocusDriver();
		}
	}
}

float FDebugOverlayFocusRuntimeHelper::GetNearestFocusRadius()
{
	return FMath::Max(0.f, CVarDebugOverlayNearestFocusRadius.GetValueOnGameThread());
}

bool FDebugOverlayFocusRuntimeHelper::TryFocusNearestFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InRadius)
{
	if (!IsValid(InFocusComponent))
	{
		return FDebugOverlayFocusLogHelper::LogInvalidFocusComponent(TEXT("DebugOverlaySelectNearestFocus"));
	}

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveNearestFocus(InWorld, InViewerPawn, InRadius);
	ApplyDebugOverlayFocusResolveResult(InFocusComponent, result, EDebugOverlayFocusDriver::ManualNearest, true, true);
	InFocusComponent->ClearDebugOverlayRecentFocusState();

	return FDebugOverlayFocusLogHelper::LogResolveResult(TEXT("DebugOverlaySelectNearestFocus"), EDebugOverlayFocusResolveLogProfile::Nearest, result);
}

bool FDebugOverlayFocusRuntimeHelper::TryFocusOutlinerFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, const FString& InActorName)
{
	const FString actorName = InActorName.TrimStartAndEnd();

	if (!IsValid(InFocusComponent))
	{
		return FDebugOverlayFocusLogHelper::LogInvalidFocusComponent(TEXT("DebugOverlaySelectOutlinerFocus"), &actorName);
	}

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveOutlinerFocus(InWorld, InViewerPawn, actorName);
	ApplyDebugOverlayFocusResolveResult(InFocusComponent, result, EDebugOverlayFocusDriver::ManualOutliner, true, true);
	InFocusComponent->ClearDebugOverlayRecentFocusState();

	return FDebugOverlayFocusLogHelper::LogResolveResult(TEXT("DebugOverlaySelectOutlinerFocus"), EDebugOverlayFocusResolveLogProfile::Outliner, result);
}

bool FDebugOverlayFocusRuntimeHelper::TryFocusRecentCombatFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius)
{
	if (!IsValid(InFocusComponent))
	{
		return FDebugOverlayFocusLogHelper::LogInvalidFocusComponent(TEXT("DebugOverlaySelectRecentCombatFocus"));
	}

	InFocusComponent->SetDebugOverlayFocusDriver(EDebugOverlayFocusDriver::RecentCombatLive);

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveRecentCombatFocus(InWorld, InViewerPawn, InFallbackRadius);
	ApplyDebugOverlayFocusResolveResult(InFocusComponent, result, EDebugOverlayFocusDriver::RecentCombatLive, false, false);
	ApplyRecentCombatOutcomePolicy(InFocusComponent, result.Outcome);

	return FDebugOverlayFocusLogHelper::LogResolveResult(TEXT("DebugOverlaySelectRecentCombatFocus"), EDebugOverlayFocusResolveLogProfile::RecentCombat, result);
}

void FDebugOverlayFocusRuntimeHelper::UpdateFocusRecentCombatFocus(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius)
{
	if (!IsValid(InFocusComponent)) return;
	if (InFocusComponent->GetDebugOverlayFocusDriver() != EDebugOverlayFocusDriver::RecentCombatLive) return;

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveRecentCombatFocus(InWorld, InViewerPawn, InFallbackRadius);
	ApplyDebugOverlayFocusResolveResult(InFocusComponent, result, EDebugOverlayFocusDriver::RecentCombatLive, false, false);
	ApplyRecentCombatOutcomePolicy(InFocusComponent, result.Outcome);
}

void FDebugOverlayFocusRuntimeHelper::ClearFocus(UCDebugOverlayFocusComponent* InFocusComponent)
{
	if (!IsValid(InFocusComponent)) return;

	InFocusComponent->ClearDebugOverlayFocusDriver();
	InFocusComponent->ClearDebugOverlayFocusActorAndSource();
	InFocusComponent->ClearDebugOverlayRecentFocusState();
}
