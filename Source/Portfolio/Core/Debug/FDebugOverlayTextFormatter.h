#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

class FDebugOverlayTextFormatter
{
public:
	static FDebugOverlayTextPanels Format(const FDebugOverlayViewData& InViewData);
};
