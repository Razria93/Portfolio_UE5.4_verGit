#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Type/CTargetingTypes.h"
#include "CTargetHUDWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class PORTFOLIO_API UCTargetHUDWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	// Runtime View Data
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Target HUD|Marker", meta = (AllowPrivateAccess = "true"))
	FTargetMarkerViewData TargetMarkerViewData;

public:
	// Marker Update
	void UpdateTargetMarker(const FTargetMarkerViewData& InViewData);

public:
	// Marker Query
	UFUNCTION(BlueprintPure, Category = "Target HUD|Marker")
	FTargetMarkerViewData GetTargetMarkerViewData() const { return TargetMarkerViewData; }

protected:
	// Blueprint Presentation
	UFUNCTION(BlueprintImplementableEvent, Category = "Target HUD|Marker", meta = (DisplayName = "On Target Marker Updated"))
	void BP_OnTargetMarkerUpdated(const FTargetMarkerViewData& InViewData);
};
