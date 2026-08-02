#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

class FDebugOverlayViewDataBuilder
{
public:
	static FDebugOverlayViewData Build(const FDebugOverlayViewDataBuildContext& InContext);
};
