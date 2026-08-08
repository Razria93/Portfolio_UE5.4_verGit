#pragma once

#include "CoreMinimal.h"

enum class EDebugOverlayFocusSource : uint8
{
	None,

	// Runtime auto-resolution sources.
	NearestTarget,
	RecentCombat,
	WorldScanFallback,

	// Gameplay-provided source.
	GameplayTarget,

	// Editor-provided source.
	OutlinerTarget,
};

enum class EDebugOverlayFocusDriver : uint8
{
	None,

	ManualNearest,
	ManualOutliner,
	RecentCombatLive,
	TargetComponentLive,
};

enum class EDebugOverlayRecentFocusState : uint8
{
	None,
	Selected,
	NoTargetFound,
	NoRecentCombatEvidence,
	ClosestOutOfRange,
};
