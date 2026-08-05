#include "Core/Debug/FDebugOverlayFocusLogHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugOverlayFocus, Log, All);

bool FDebugOverlayFocusLogHelper::LogInvalidTargetComponent(const TCHAR* InCommandName, const FString* InActorName)
{
	if (InActorName)
	{
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: InvalidTargetComponent | Name: %s"), InCommandName, **InActorName);
		return false;
	}

	UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: InvalidTargetComponent"), InCommandName);
	return false;
}

bool FDebugOverlayFocusLogHelper::LogResolveResult(const TCHAR* InCommandName, EDebugOverlayFocusResolveLogProfile InProfile, const FDebugOverlayFocusResolveResult& InResult)
{
	switch (InResult.Outcome)
	{
		case EDebugOverlayFocusResolveOutcome::InvalidContext:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: InvalidContext"), InCommandName);
		return false;
		case EDebugOverlayFocusResolveOutcome::NoTarget:
		if (InProfile == EDebugOverlayFocusResolveLogProfile::RecentCombat)
		{
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: DriverEnabled | NoTargetFound"), InCommandName);
			return true;
		}

		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoTarget | Radius: %.0f"), InCommandName, InResult.Radius);
		return false;
		case EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence:
		if (InProfile == EDebugOverlayFocusResolveLogProfile::RecentCombat)
		{
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: DriverEnabled | NoRecentCombatEvidence"), InCommandName);
			return true;
		}

		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoRecentCombatEvidence"), InCommandName);
		return false;
		case EDebugOverlayFocusResolveOutcome::OutOfRange:
		if (InProfile == EDebugOverlayFocusResolveLogProfile::RecentCombat)
		{
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: DriverEnabled | ClosestOutOfRange | Closest: %.0f | Radius: %.0f"), InCommandName, InResult.Distance, InResult.Radius);
			return true;
		}

		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: OutOfRange | Closest: %.0f | Radius: %.0f"), InCommandName, InResult.Distance, InResult.Radius);
		return false;
		case EDebugOverlayFocusResolveOutcome::NoActorName:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoActorName"), InCommandName);
		return false;
		case EDebugOverlayFocusResolveOutcome::NoActor:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoActor | Name: %s"), InCommandName, *InResult.ActorName);
		return false;
		case EDebugOverlayFocusResolveOutcome::TargetIsNotEnemy:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: TargetIsNotEnemy | Target: %s | Class: %s"), InCommandName, *InResult.ActorName, *InResult.ClassName);
		return false;
		case EDebugOverlayFocusResolveOutcome::Selected:
		if (InProfile == EDebugOverlayFocusResolveLogProfile::Nearest)
		{
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: Selected | Target: %s | Distance: %.0f | Radius: %.0f"), InCommandName, *InResult.ActorName, InResult.Distance, InResult.Radius);
			return true;
		}

		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: Selected | Target: %s"), InCommandName, *InResult.ActorName);
		return true;
	default:
		return false;
	}
}
