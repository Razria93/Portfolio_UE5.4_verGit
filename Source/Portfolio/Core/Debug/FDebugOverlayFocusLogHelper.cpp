#include "Core/Debug/FDebugOverlayFocusLogHelper.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugOverlayFocus, Log, All);

namespace
{
	bool TryLogRecentCombatResolveResult(const TCHAR* InCommandName, const FDebugOverlayFocusResolveResult& InResult, bool& bOutReturnValue)
	{
		switch (InResult.Outcome)
		{
		case EDebugOverlayFocusResolveOutcome::NoTarget:
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: DriverEnabled | NoFocusFound"), InCommandName);
			bOutReturnValue = true;
			return true;
		case EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence:
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: DriverEnabled | NoRecentCombatEvidence"), InCommandName);
			bOutReturnValue = true;
			return true;
		case EDebugOverlayFocusResolveOutcome::OutOfRange:
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: DriverEnabled | ClosestOutOfRange | Closest: %.0f | Radius: %.0f"), InCommandName, InResult.Distance, InResult.Radius);
			bOutReturnValue = true;
			return true;
		default:
			return false;
		}
	}

	bool LogSelectedResolveResult(const TCHAR* InCommandName, EDebugOverlayFocusResolveLogProfile InProfile, const FDebugOverlayFocusResolveResult& InResult)
	{
		if (InProfile == EDebugOverlayFocusResolveLogProfile::Nearest)
		{
			UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: Selected | Focus: %s | Distance: %.0f | Radius: %.0f"), InCommandName, *InResult.ActorName, InResult.Distance, InResult.Radius);
			return true;
		}

		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: Selected | Focus: %s"), InCommandName, *InResult.ActorName);
		return true;
	}
}

bool FDebugOverlayFocusLogHelper::LogInvalidFocusComponent(const TCHAR* InCommandName, const FString* InActorName)
{
	if (InActorName)
	{
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: InvalidFocusComponent | Name: %s"), InCommandName, **InActorName);
		return false;
	}

	UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: InvalidFocusComponent"), InCommandName);
	return false;
}

bool FDebugOverlayFocusLogHelper::LogResolveResult(const TCHAR* InCommandName, EDebugOverlayFocusResolveLogProfile InProfile, const FDebugOverlayFocusResolveResult& InResult)
{
	if (InProfile == EDebugOverlayFocusResolveLogProfile::RecentCombat)
	{
		bool bRecentCombatReturnValue = false;
		if (TryLogRecentCombatResolveResult(InCommandName, InResult, bRecentCombatReturnValue))
		{
			return bRecentCombatReturnValue;
		}
	}

	switch (InResult.Outcome)
	{
	case EDebugOverlayFocusResolveOutcome::InvalidContext:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: InvalidContext"), InCommandName);
		return false;
	case EDebugOverlayFocusResolveOutcome::NoTarget:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoTarget | Radius: %.0f"), InCommandName, InResult.Radius);
		return false;
	case EDebugOverlayFocusResolveOutcome::NoRecentCombatEvidence:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoRecentCombatEvidence"), InCommandName);
		return false;
	case EDebugOverlayFocusResolveOutcome::OutOfRange:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: OutOfRange | Closest: %.0f | Radius: %.0f"), InCommandName, InResult.Distance, InResult.Radius);
		return false;
	case EDebugOverlayFocusResolveOutcome::NoActorName:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoActorName"), InCommandName);
		return false;
	case EDebugOverlayFocusResolveOutcome::NoActor:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: NoActor | Name: %s"), InCommandName, *InResult.ActorName);
		return false;
	case EDebugOverlayFocusResolveOutcome::FocusActorIsNotEnemy:
		UE_LOG(LogDebugOverlayFocus, Log, TEXT("%s Result: FocusActorIsNotEnemy | Focus: %s | Class: %s"), InCommandName, *InResult.ActorName, *InResult.ClassName);
		return false;
	case EDebugOverlayFocusResolveOutcome::Selected:
		return LogSelectedResolveResult(InCommandName, InProfile, InResult);
	default:
		return false;
	}
}
