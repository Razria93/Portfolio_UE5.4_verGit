#pragma once

#include "CoreMinimal.h"

enum class EDebugOverlayFocusSource : uint8
{
	None,

	// Runtime auto-resolution sources.
	NearestFocus,
	RecentCombatFocus,
	WorldScanFallback,
	PlayerTargetFocus,

	// Gameplay-provided source.
	GameplayFocus,

	// Editor-provided source.
	OutlinerFocus,
};

enum class EDebugOverlayFocusDriver : uint8
{
	None,

	ManualNearest,
	ManualOutliner,
	RecentCombatLive,
	PlayerTargetLive,
	PlayerTargetFrozen,
	FocusComponentLive,
};

enum class EDebugOverlayRecentFocusState : uint8
{
	None,
	Selected,
	NoFocusFound,
	NoRecentCombatEvidence,
	ClosestOutOfRange,
};
