#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "CPixelAlignedGaugeImage.generated.h"

// Axis-aligned HUD grid cell. Snap the grid pitch, not each cell independently.
UCLASS()
class PORTFOLIO_API UCPixelAlignedGaugeImage : public UImage
{
	GENERATED_BODY()
public:
	int32 Column = 0;
	int32 Row = 0;
	int32 UnitColumns = 0;
	bool bBossGrid = false;
	bool bBossOffsetOnly = false;
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
