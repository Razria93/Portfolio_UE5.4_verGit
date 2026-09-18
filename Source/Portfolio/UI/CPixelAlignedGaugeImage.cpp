#include "UI/CPixelAlignedGaugeImage.h"
#include "UI/CCombatHUDGaugeLayout.h"
#include "Widgets/Images/SImage.h"

class SPixelAlignedGaugeImage : public SImage
{
public:
	int32 Column = 0;
	int32 Row = 0;
	int32 UnitColumns = 0;
	bool bBossGrid = false;
	bool bBossOffsetOnly = false;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
		const FSlateRect& CullingRect, FSlateWindowElementList& Elements, int32 Layer,
		const FWidgetStyle& Style, bool bParentEnabled) const override
	{
		const FVector2D origin = Geometry.LocalToAbsolute(FVector2D::ZeroVector);
		if (bBossOffsetOnly)
		{
			const float scale = Geometry.GetAccumulatedLayoutTransform().GetScale();
			const CombatHUDGauge::FPixelMetrics pixels(scale);
			const float offsetX = FMath::RoundToFloat((880.f * scale - (pixels.ColumnX(CombatHUDGauge::BossHealthColumns - 1, 0) + pixels.Cell)) / 2.f);
			const FVector2D offset = Geometry.AbsoluteToLocal(origin + FVector2D(offsetX, 0));
			return SImage::OnPaint(Args, Geometry.MakeChild(Geometry.GetLocalSize(), FSlateLayoutTransform(offset)), CullingRect, Elements, Layer, Style, bParentEnabled);
		}
		const FVector2D axis = Geometry.LocalToAbsolute(FVector2D(1, 1)) - origin;
		if (axis.X <= 0.f || axis.Y <= 0.f)
			return SImage::OnPaint(Args, Geometry, CullingRect, Elements, Layer, Style, bParentEnabled);
		const CombatHUDGauge::FPixelMetrics x(axis.X), y(axis.Y);
		const FVector2D gridOrigin = origin - FVector2D(
			CombatHUDGauge::ColumnX(Column, UnitColumns, bBossGrid) * axis.X,
			Row * (CombatHUDGauge::CellSize + CombatHUDGauge::CellGap) * axis.Y);
		const float centerOffset = bBossGrid ? (880.f * axis.X - (x.ColumnX(CombatHUDGauge::BossHealthColumns - 1, 0) + x.Cell)) / 2.f : 0.f;
		const FVector2D snappedOrigin(FMath::RoundToFloat(gridOrigin.X + centerOffset) + x.ColumnX(Column, UnitColumns, bBossGrid),
			FMath::RoundToFloat(gridOrigin.Y) + Row * (y.Cell + y.Gap));
		// A partial health column keeps its fractional coverage; full cells are integer-sized.
		const float coverage = FMath::Clamp(Geometry.GetLocalSize().X / CombatHUDGauge::CellSize, 0.f, 1.f);
		const FVector2D size(x.Cell * coverage / axis.X, y.Cell / axis.Y);
		const FVector2D offset = Geometry.AbsoluteToLocal(snappedOrigin);
		const FGeometry snapped = Geometry.MakeChild(size, FSlateLayoutTransform(offset));
		return SImage::OnPaint(Args, snapped, CullingRect, Elements, Layer, Style, bParentEnabled);
	}
};

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
