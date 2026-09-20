#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "CPixelAlignedGaugeImage.generated.h"

UCLASS()
class PORTFOLIO_API UCPixelAlignedGaugeImage : public UImage
{
	GENERATED_BODY()

public:
	// Grid Coordinates
	int32 Column = 0;
	int32 Row = 0;

	// Grid Layout
	int32 UnitColumns = 0;

	// Boss Alignment
	bool bBossGrid = false;
	bool bBossOffsetOnly = false;

protected:
	// Widget Lifecycle
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
