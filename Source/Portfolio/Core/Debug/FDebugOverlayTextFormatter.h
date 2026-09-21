#pragma once

#include "CoreMinimal.h"
#include "Core/Debug/FDebugOverlayTextPanelTypes.h"
#include "Core/Debug/FDebugOverlayViewDataTypes.h"

class PORTFOLIO_API FDebugOverlayTextFormatter
{
public:
	static FDebugOverlayTextPanels Format(const FDebugOverlayViewData& InViewData);
};
