#include "UI/CPixelAlignedGaugeImage.h"
#include "UI/CCombatHUDGaugeLayout.h"
#include "Widgets/Images/SImage.h"

class SPixelAlignedGaugeImage : public SImage
{
public:
	// Grid Coordinates
	int32 Column = 0;
	int32 Row = 0;

	// Grid Layout
	int32 UnitColumns = 0;

	// Boss Alignment
	bool bBossGrid = false;
	bool bBossOffsetOnly = false;

public:
	// Painting
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
		const FSlateRect& CullingRect, FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle& Style, bool bParentEnabled) const override
	{
		if (bBossOffsetOnly)
		{
			const FGeometry adjusted = BuildBossOffsetGeometry(Geometry);
			return SImage::OnPaint(Args, adjusted, CullingRect, Elements, Layer, Style, bParentEnabled);
		}

		const FVector2D origin = Geometry.LocalToAbsolute(FVector2D::ZeroVector);
		const FVector2D axisScale = Geometry.LocalToAbsolute(FVector2D(1, 1)) - origin;

		if (axisScale.X <= 0.f || axisScale.Y <= 0.f)
			return SImage::OnPaint(Args, Geometry, CullingRect, Elements, Layer, Style, bParentEnabled);

		const FGeometry adjusted = BuildPixelAlignedGeometry(Geometry, origin, axisScale);
		return SImage::OnPaint(Args, adjusted, CullingRect, Elements, Layer, Style, bParentEnabled);
	}

private:
	// Paint Geometry
	FGeometry BuildBossOffsetGeometry(const FGeometry& Geometry) const
	{
		const FVector2D origin = Geometry.LocalToAbsolute(FVector2D::ZeroVector);
		const float scale = Geometry.GetAccumulatedLayoutTransform().GetScale();

		const CombatHUDGauge::FPixelMetrics pixels(scale);

		const int32 gridPixelWidth = pixels.ColumnX(CombatHUDGauge::BossHealthColumns - 1, 0) + pixels.Cell;
		const float offsetX = FMath::RoundToFloat((CombatHUDGauge::BossWidth * scale - gridPixelWidth) / 2.f);
		const FVector2D localOffset = Geometry.AbsoluteToLocal(origin + FVector2D(offsetX, 0));

		return Geometry.MakeChild(Geometry.GetLocalSize(), FSlateLayoutTransform(localOffset));
	}

	FGeometry BuildPixelAlignedGeometry(const FGeometry& Geometry, const FVector2D& Origin, const FVector2D& AxisScale) const
	{
		const CombatHUDGauge::FPixelMetrics xPixels(AxisScale.X);
		const CombatHUDGauge::FPixelMetrics yPixels(AxisScale.Y);

		const FVector2D gridOrigin = Origin - FVector2D(
			CombatHUDGauge::ColumnX(Column, UnitColumns, bBossGrid) * AxisScale.X,
			Row * (CombatHUDGauge::CellSize + CombatHUDGauge::CellGap) * AxisScale.Y);
		const int32 gridPixelWidth = xPixels.ColumnX(CombatHUDGauge::BossHealthColumns - 1, 0) + xPixels.Cell;
		const float centerOffset = bBossGrid ? (CombatHUDGauge::BossWidth * AxisScale.X - gridPixelWidth) / 2.f : 0.f;

		const FVector2D snappedOrigin(
			FMath::RoundToFloat(gridOrigin.X + centerOffset) + xPixels.ColumnX(Column, UnitColumns, bBossGrid),
			FMath::RoundToFloat(gridOrigin.Y) + Row * (yPixels.Cell + yPixels.Gap));

		const float coverage = FMath::Clamp(Geometry.GetLocalSize().X / CombatHUDGauge::CellSize, 0.f, 1.f);
		const FVector2D localSize(xPixels.Cell * coverage / AxisScale.X, yPixels.Cell / AxisScale.Y);
		const FVector2D localOffset = Geometry.AbsoluteToLocal(snappedOrigin);

		return Geometry.MakeChild(localSize, FSlateLayoutTransform(localOffset));
	}
};

// Widget Lifecycle

TSharedRef<SWidget> UCPixelAlignedGaugeImage::RebuildWidget()
{
	TSharedRef<SPixelAlignedGaugeImage> image = SNew(SPixelAlignedGaugeImage);

	image->Column = Column;
	image->Row = Row;

	image->UnitColumns = UnitColumns;

	image->bBossGrid = bBossGrid;
	image->bBossOffsetOnly = bBossOffsetOnly;

	MyImage = image;
	return image;
}
