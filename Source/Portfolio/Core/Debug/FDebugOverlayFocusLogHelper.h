#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlayFocusResolver.h"

enum class EDebugOverlayFocusResolveLogProfile : uint8
{
	Nearest,
	Outliner,
	RecentCombat,
};

class FDebugOverlayFocusLogHelper
{
public:
	static bool LogInvalidTargetComponent(const TCHAR* InCommandName, const FString* InActorName = nullptr);
	static bool LogResolveResult(const TCHAR* InCommandName, EDebugOverlayFocusResolveLogProfile InProfile, const FDebugOverlayFocusResolveResult& InResult);
};
