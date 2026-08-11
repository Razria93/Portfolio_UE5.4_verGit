#include "UI/CTargetHUDWidget.h"

// ===== Marker Update =====

void UCTargetHUDWidget::UpdateTargetMarker(const FTargetMarkerViewData& InViewData)
{
	TargetMarkerViewData = InViewData;
	BP_OnTargetMarkerUpdated(TargetMarkerViewData);
}
