#include "Core/Debug/FDebugOverlayFocusRuntimeHelper.h"

#include "Core/Debug/CDebugOverlayFocusComponent.h"
#include "Core/Debug/FDebugOverlayFocusLogHelper.h"
#include "Core/Debug/FDebugOverlayFocusResolver.h"
#include "HAL/IConsoleManager.h"

namespace
{
	// ===== CVars =====

	static TAutoConsoleVariable<float> CVarDebugOverlayNearestTargetRadius(
		TEXT("Portfolio.DebugOverlay.NearestTargetRadius"),
		3000.f,
		TEXT("Nearest target search radius used by Debug Overlay focus commands."),
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
			return EDebugOverlayRecentFocusState::NoTargetFound;
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

float FDebugOverlayFocusRuntimeHelper::GetNearestTargetRadius()
{
	return FMath::Max(0.f, CVarDebugOverlayNearestTargetRadius.GetValueOnGameThread());
}

bool FDebugOverlayFocusRuntimeHelper::TryFocusNearestTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InRadius)
{
	if (!IsValid(InFocusComponent))
	{
		return FDebugOverlayFocusLogHelper::LogInvalidTargetComponent(TEXT("DebugOverlaySelectNearestTarget"));
	}

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveNearestTarget(InWorld, InViewerPawn, InRadius);
	ApplyDebugOverlayFocusResolveResult(InFocusComponent, result, EDebugOverlayFocusDriver::ManualNearest, true, true);
	InFocusComponent->ClearDebugOverlayRecentFocusState();

	return FDebugOverlayFocusLogHelper::LogResolveResult(TEXT("DebugOverlaySelectNearestTarget"), EDebugOverlayFocusResolveLogProfile::Nearest, result);
}

bool FDebugOverlayFocusRuntimeHelper::TryFocusOutlinerTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, const FString& InActorName)
{
	const FString actorName = InActorName.TrimStartAndEnd();

	if (!IsValid(InFocusComponent))
	{
		return FDebugOverlayFocusLogHelper::LogInvalidTargetComponent(TEXT("DebugOverlaySelectOutlinerTarget"), &actorName);
	}

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveOutlinerTarget(InWorld, InViewerPawn, actorName);
	ApplyDebugOverlayFocusResolveResult(InFocusComponent, result, EDebugOverlayFocusDriver::ManualOutliner, true, true);
	InFocusComponent->ClearDebugOverlayRecentFocusState();

	return FDebugOverlayFocusLogHelper::LogResolveResult(TEXT("DebugOverlaySelectOutlinerTarget"), EDebugOverlayFocusResolveLogProfile::Outliner, result);
}

bool FDebugOverlayFocusRuntimeHelper::TryFocusRecentCombatTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius)
{
	if (!IsValid(InFocusComponent))
	{
		return FDebugOverlayFocusLogHelper::LogInvalidTargetComponent(TEXT("DebugOverlaySelectRecentCombatTarget"));
	}

	InFocusComponent->SetDebugOverlayFocusDriver(EDebugOverlayFocusDriver::RecentCombatLive);

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveRecentCombatTarget(InWorld, InViewerPawn, InFallbackRadius);
	ApplyDebugOverlayFocusResolveResult(InFocusComponent, result, EDebugOverlayFocusDriver::RecentCombatLive, false, false);
	ApplyRecentCombatOutcomePolicy(InFocusComponent, result.Outcome);

	return FDebugOverlayFocusLogHelper::LogResolveResult(TEXT("DebugOverlaySelectRecentCombatTarget"), EDebugOverlayFocusResolveLogProfile::RecentCombat, result);
}

void FDebugOverlayFocusRuntimeHelper::UpdateFocusRecentCombatTarget(UCDebugOverlayFocusComponent* InFocusComponent, UWorld* InWorld, const APawn* InViewerPawn, float InFallbackRadius)
{
	if (!IsValid(InFocusComponent)) return;
	if (InFocusComponent->GetDebugOverlayFocusDriver() != EDebugOverlayFocusDriver::RecentCombatLive) return;

	const FDebugOverlayFocusResolveResult result = FDebugOverlayFocusResolver::ResolveRecentCombatTarget(InWorld, InViewerPawn, InFallbackRadius);
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
