#include "UI/CCombatHUDWidget.h"
#include "UI/CCombatHUDGaugeLayout.h"
#include "UI/CPixelAlignedGaugeImage.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Brushes/SlateRoundedBoxBrush.h"

namespace
{
	const FLinearColor Ink(0.9f, 0.91f, 0.86f, 1.f);
	const FLinearColor Empty(0.12f, 0.17f, 0.19f, 0.65f);
	const FLinearColor Pending(0.30f, 0.36f, 0.39f, 0.55f);
	void Place(UCanvasPanel* Parent, UWidget* Child, FVector2D Position, FVector2D Size)
	{
		UCanvasPanelSlot* slot = Parent->AddChildToCanvas(Child);
		slot->SetPosition(Position);
		slot->SetSize(Size);
	}
	void PaintHealth(const TArray<TObjectPtr<UImage>>& Cells, const FHUDResourceViewData& Value, bool bFixedUnits = false)
	{
		const int32 columns = Cells.Num() / CombatHUDGauge::HealthRows;
		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			const float coverage = bFixedUnits
				? (Value.Availability == EHUDResourceAvailability::Available ? CombatHUDGauge::UnitFill(Value.Current, Value.Maximum, CombatHUDGauge::PlayerHealthPerColumn, i / CombatHUDGauge::HealthRows) : 0.f)
				: CombatHUDGauge::ColumnFill(Value.GetFraction(), i / CombatHUDGauge::HealthRows, columns);
			CastChecked<UCanvasPanelSlot>(Cells[i]->Slot)->SetSize(FVector2D(CombatHUDGauge::CellSize * coverage, CombatHUDGauge::CellSize));
			Cells[i]->SetVisibility(coverage > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}
}

UCCombatHUDWidget::UCCombatHUDWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UFont> label(TEXT("/Game/08_UI/CombatHUD/F_HUDLabel"));
	static ConstructorHelpers::FObjectFinder<UFont> name(TEXT("/Game/08_UI/CombatHUD/F_HUDName"));
	LabelFont = label.Object;
	NameFont = name.Object;
	const TCHAR* paths[] = {
		TEXT("/Game/08_UI/CombatHUD/T_HUDSlash"), TEXT("/Game/08_UI/CombatHUD/T_HUDSweep"),
		TEXT("/Game/08_UI/CombatHUD/T_HUDShield"), TEXT("/Game/08_UI/CombatHUD/T_HUDStrike") };
	for (const TCHAR* path : paths)
	{
		ConstructorHelpers::FObjectFinder<UTexture2D> icon(path);
		SkillIcons.Add(icon.Object);
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> vial(TEXT("/Game/08_UI/CombatHUD/T_HUDVial"));
	ItemIcon = vial.Object;
}

void UCCombatHUDWidget::AddSlotFrame(UCanvasPanel* Parent, FVector2D Position, float Size, float Radius)
{
	UImage* frame = AddImage(Parent, Position, FVector2D(Size), FLinearColor::White);
	frame->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.015f, 0.02f, 0.025f, 0.25f), Radius,
		FLinearColor(0.62f, 0.67f, 0.69f, 0.6f), 1.f));
}

UImage* UCCombatHUDWidget::AddImage(UCanvasPanel* Parent, FVector2D Position, FVector2D Size, FLinearColor Color)
{
	UImage* image = WidgetTree->ConstructWidget<UImage>();
	image->SetBrush(*FCoreStyle::Get().GetBrush("WhiteBrush"));
	image->SetColorAndOpacity(Color);
	Place(Parent, image, Position, Size);
	return image;
}

UTextBlock* UCCombatHUDWidget::AddLabel(UCanvasPanel* Parent, const FString& Text, FVector2D Position, FVector2D Size, int32 FontSize)
{
	UTextBlock* label = WidgetTree->ConstructWidget<UTextBlock>();
	label->SetText(FText::FromString(Text));
	FSlateFontInfo font = FCoreStyle::GetDefaultFontStyle("Regular", FontSize);
	if (LabelFont) font.FontObject = LabelFont;
	label->SetFont(font);
	label->SetColorAndOpacity(Ink);
	label->SetShadowOffset(FVector2D(1.f, 1.f));
	label->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f));
	label->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	Place(Parent, label, Position, Size);
	return label;
}

void UCCombatHUDWidget::AddGauge(UCanvasPanel* Parent, FVector2D Position, float Width, int32 Rows, int32 UnitColumns,
	TArray<TObjectPtr<UImage>>& Cells, bool bUnimplemented, int32 ExplicitColumns, bool bBoss)
{
	const int32 columns = ExplicitColumns >= 0 ? ExplicitColumns : CombatHUDGauge::ColumnsForWidth(Width, UnitColumns);
	for (int32 column = 0; column < columns; ++column)
	{
		for (int32 row = 0; row < Rows; ++row)
		{
			const FVector2D position = Position + FVector2D(CombatHUDGauge::ColumnX(column, UnitColumns, bBoss), row * (CombatHUDGauge::CellSize + CombatHUDGauge::CellGap));
			auto addCell = [&](FLinearColor Color) -> UImage*
			{
				UCPixelAlignedGaugeImage* image = WidgetTree->ConstructWidget<UCPixelAlignedGaugeImage>();
				image->Column = column;
				image->Row = row;
				image->UnitColumns = UnitColumns;
				image->bBossGrid = bBoss;
				image->SetBrush(*FCoreStyle::Get().GetBrush("WhiteBrush"));
				image->SetColorAndOpacity(Color);
				Place(Parent, image, position, FVector2D(CombatHUDGauge::CellSize));
				return image;
			};
			UImage* background = addCell(Empty);
			if (bUnimplemented)
			{
				background->SetBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f, Pending, 0.75f));
				background->SetColorAndOpacity(FLinearColor::White);
			}
			else
				Cells.Add(addCell(Ink));
		}
	}
}

TSharedRef<SWidget> UCCombatHUDWidget::RebuildWidget()
{
	PlayerCells.Reset();
	PlayerHealthColumns = -1;
	LastPlayerMaximum = -1.f;
	TargetCells.Reset();
	BalanceCells.Reset();
	UScaleBox* scale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("HUDScale"));
	scale->SetStretch(EStretch::ScaleToFit);
	USizeBox* size = WidgetTree->ConstructWidget<USizeBox>();
	size->SetWidthOverride(2560.f);
	size->SetHeightOverride(1440.f);
	scale->AddChild(size);
	UCanvasPanel* canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	size->AddChild(canvas);
	WidgetTree->RootWidget = scale;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	TargetPanel = WidgetTree->ConstructWidget<UCanvasPanel>();
	Place(canvas, TargetPanel, FVector2D(840, 48), FVector2D(880, 132));
	TargetName = AddLabel(TargetPanel, TEXT("TARGET"), FVector2D(0, 0), FVector2D(880, 36), 21);
	TargetName->SetJustification(ETextJustify::Center);
	if (NameFont)
	{
		FSlateFontInfo font(NameFont.Get(), 21);
		font.LetterSpacing = 180;
		TargetName->SetFont(font);
	}
	bool showSamples = false;
#if WITH_EDITOR
	showSamples = bShowResourceSamples;
#endif
	AddGauge(TargetPanel, FVector2D(0, 47), 880.f, CombatHUDGauge::HealthRows, 0, TargetCells, false, CombatHUDGauge::BossHealthColumns, true);
	TargetHealthValue = AddLabel(TargetPanel, TEXT("\u2014"), FVector2D(888, 42), FVector2D(260, 28), 15);
	TargetBalanceValue = AddLabel(TargetPanel, TEXT("\u2014"), FVector2D(888, 99), FVector2D(260, 28), 15);
	TArray<TObjectPtr<UImage>> unused;
	AddGauge(TargetPanel, FVector2D(0, 82), 880.f, 2, CombatHUDGauge::BossUnitColumns, unused, !showSamples,
		CombatHUDGauge::BossUnitColumns * CombatHUDGauge::BossGroups, true);
	UTextBlock* shieldValue = AddLabel(TargetPanel, TEXT("\u2014"), FVector2D(888, 73), FVector2D(260, 28), 15);
	if (showSamples)
	{
		FHUDResourceViewData sample;
		sample.Availability = EHUDResourceAvailability::Available;
		sample.Current = 2700.f;
		sample.Maximum = 4500.f;
		shieldValue->SetText(sample.GetValueText());
		AddLabel(TargetPanel, TEXT("SAMPLE: SH"), FVector2D(888, 128), FVector2D(200, 22), 10);
		for (int32 i = 0; i < unused.Num(); ++i)
		{
			const float fill = CombatHUDGauge::ColumnFill(sample.GetFraction(), i / 2, unused.Num() / 2);
			unused[i]->SetColorAndOpacity(FLinearColor(0.35f, 0.8f, 0.65f));
			CastChecked<UCanvasPanelSlot>(unused[i]->Slot)->SetSize(FVector2D(CombatHUDGauge::CellSize * fill, CombatHUDGauge::CellSize));
			unused[i]->SetVisibility(fill > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}

	PlayerPanel = WidgetTree->ConstructWidget<UCanvasPanel>();
	Place(canvas, PlayerPanel, FVector2D(64, 1208), FVector2D(1000, 160));
	AddImage(PlayerPanel, FVector2D(108, 0), FVector2D(2.67f, 128), FLinearColor(0.62f, 0.67f, 0.69f, 0.6f));
	const TCHAR* labels[] = { TEXT("BU"), TEXT("BE"), TEXT("SH"), TEXT("HP") };
	if (showSamples)
		AddLabel(PlayerPanel, TEXT("SAMPLE: BU / BE / SH"), FVector2D(182, -30), FVector2D(320, 22), 10);
	// Align visible BU top / HP bottom with the two item frames (0..128).
	const float firstRowY = CombatHUDGauge::Height(2) / 2.f - 13.f;
	const float lastRowY = 128.f - CombatHUDGauge::Height(3) / 2.f - 14.f;
	for (int32 row = 0; row < 4; ++row)
	{
		const float y = FMath::Lerp(firstRowY, lastRowY, row / 3.f);
		const float gaugeCenterY = y + (row == 3 ? 14.f : 13.f);
		// Optical centers of the approved Oxanium glyphs, excluding line-box padding.
		AddLabel(PlayerPanel, labels[row], FVector2D(126, gaugeCenterY - 16.f), FVector2D(48, 30), 17);
		if (row == 3)
		{
			PlayerHealthGrid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PlayerHealthGrid"));
			Place(PlayerPanel, PlayerHealthGrid, FVector2D(182, gaugeCenterY - CombatHUDGauge::Height(3) / 2.f), FVector2D(580, CombatHUDGauge::Height(3)));
			PlayerHealthValue = AddLabel(PlayerPanel, TEXT("—"), FVector2D(778, gaugeCenterY - 15.f), FVector2D(220, 28), 15);
		}
		else
		{
			unused.Reset();
			const float current[] = { 1200.f, 700.f, 560.f };
			const float maximum[] = { 1600.f, 1400.f, 800.f };
			const int32 unitColumns = row == 2 ? CombatHUDGauge::ShieldUnitColumns : 2;
			const int32 columns = CombatHUDGauge::ResourceColumns(maximum[row], row == 2 ? CombatHUDGauge::ShieldPerColumn : 2.f * CombatHUDGauge::EnergyPerCell, 64);
			const float width = CombatHUDGauge::ColumnX(columns - 1, unitColumns) + CombatHUDGauge::CellSize;
			AddGauge(PlayerPanel, FVector2D(182, gaugeCenterY - CombatHUDGauge::Height(2) / 2.f), 320.f, 2, unitColumns, unused, !showSamples, columns);
			if (showSamples)
			{
				const FLinearColor colors[] = { FLinearColor(0.9f, 0.35f, 0.25f), FLinearColor(0.3f, 0.72f, 0.9f), FLinearColor(0.35f, 0.8f, 0.65f) };
				FHUDResourceViewData sample;
				sample.Availability = EHUDResourceAvailability::Available;
				sample.Current = current[row];
				sample.Maximum = maximum[row];
				for (int32 i = 0; i < unused.Num(); ++i)
				{
					const float fill = row == 2
						? CombatHUDGauge::UnitFill(sample.Current, sample.Maximum, CombatHUDGauge::ShieldPerColumn, i / 2)
						: CombatHUDGauge::UnitFill(sample.Current, sample.Maximum, CombatHUDGauge::EnergyPerCell, i);
					FLinearColor color = colors[row];
					if (row != 2) color.A = fill;
					unused[i]->SetColorAndOpacity(color);
					CastChecked<UCanvasPanelSlot>(unused[i]->Slot)->SetSize(FVector2D(CombatHUDGauge::CellSize * (row == 2 ? fill : 1.f), CombatHUDGauge::CellSize));
					unused[i]->SetVisibility(fill > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
				}
				UTextBlock* value = AddLabel(PlayerPanel, TEXT(""), FVector2D(182 + width + 16, gaugeCenterY - 15.f), FVector2D(220, 28), 15);
				value->SetText(sample.GetValueText());
			}
			else
				AddLabel(PlayerPanel, TEXT("—"), FVector2D(512, gaugeCenterY - 17.f), FVector2D(32, 30), 17);
		}
	}
	for (int32 i = 0; i < 2; ++i)
	{
		AddSlotFrame(PlayerPanel, FVector2D(0, i * 72.f), 56.f, 5.f);
		UImage* icon = AddImage(PlayerPanel, FVector2D(0, i * 72.f), FVector2D(56, 56), Pending);
		if (ItemIcon) icon->SetBrushFromTexture(ItemIcon);
		AddLabel(PlayerPanel, TEXT("—"), FVector2D(66, i * 72.f + 14), FVector2D(28, 28), 17);
	}
	AddLabel(PlayerPanel, TEXT("TODO"), FVector2D(0, 136), FVector2D(56, 24), 11)->SetJustification(ETextJustify::Center);

	SkillsPanel = WidgetTree->ConstructWidget<UCanvasPanel>();
	Place(canvas, SkillsPanel, FVector2D(2220, 1100), FVector2D(276, 268));
	const FVector2D positions[] = { FVector2D(92, 30), FVector2D(0, 112), FVector2D(184, 112), FVector2D(92, 194) };
	for (int32 i = 0; i < 4; ++i)
	{
		AddSlotFrame(SkillsPanel, positions[i], 72.f, 36.f);
		AddSlotFrame(SkillsPanel, positions[i] + FVector2D(68, 52), 24.f, 12.f);
		UImage* icon = AddImage(SkillsPanel, positions[i], FVector2D(72, 72), Pending);
		if (SkillIcons.IsValidIndex(i) && SkillIcons[i]) icon->SetBrushFromTexture(SkillIcons[i]);
	}
	AddLabel(SkillsPanel, TEXT("TODO"), FVector2D(92, 0), FVector2D(72, 26), 11)->SetJustification(ETextJustify::Center);
	AddSlotFrame(SkillsPanel, FVector2D(114, -38), 28.f, 14.f);
	UpdatePresentation();
	return Super::RebuildWidget();
}

void UCCombatHUDWidget::ApplyViewData(const FCombatHUDViewData& Data)
{
	ViewData = Data;
	UpdatePresentation();
}

void UCCombatHUDWidget::UpdatePresentation()
{
	if (!PlayerPanel || !TargetPanel) return;
	PlayerPanel->SetVisibility(ViewData.bHasPlayer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	SkillsPanel->SetVisibility(ViewData.bHasPlayer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	TargetPanel->SetVisibility(ViewData.bHasTarget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	TargetName->SetText(ViewData.TargetName);
	TargetHealthValue->SetText(ViewData.TargetHealth.GetValueText());
	TargetBalanceValue->SetText(ViewData.BalanceMaximum > 0
		? FText::Format(NSLOCTEXT("CombatHUD", "BalanceValue", "{0} / {1}"),
			FText::AsNumber(FMath::Clamp(ViewData.BalanceRemaining, 0, ViewData.BalanceMaximum)), FText::AsNumber(ViewData.BalanceMaximum))
		: FText::FromString(TEXT("\u2014")));
	PlayerHealthValue->SetText(ViewData.PlayerHealth.GetValueText());
	const float maximum = ViewData.PlayerHealth.Availability == EHUDResourceAvailability::Available && FMath::IsFinite(ViewData.PlayerHealth.Maximum)
		? FMath::Max(0.f, ViewData.PlayerHealth.Maximum) : 0.f;
	const int32 limit = CombatHUDGauge::ColumnsForWidth(580.f, 0);
	const int32 columns = CombatHUDGauge::ResourceColumns(maximum, CombatHUDGauge::PlayerHealthPerColumn, limit);
	if (maximum != LastPlayerMaximum)
	{
		LastPlayerMaximum = maximum;
		if (maximum > limit * CombatHUDGauge::PlayerHealthPerColumn)
			UE_LOG(LogTemp, Warning, TEXT("CombatHUD: Player HP maximum %.0f exceeds fixed-unit display capacity %.0f; numeric value remains authoritative."), maximum, limit * CombatHUDGauge::PlayerHealthPerColumn);
	}
	if (columns != PlayerHealthColumns)
	{
		PlayerHealthGrid->ClearChildren();
		PlayerCells.Reset();
		PlayerHealthColumns = columns;
		AddGauge(PlayerHealthGrid, FVector2D::ZeroVector, 580.f, 3, 0, PlayerCells, false, columns);
		UCanvasPanelSlot* valueSlot = CastChecked<UCanvasPanelSlot>(PlayerHealthValue->Slot);
		const float width = columns > 0 ? CombatHUDGauge::ColumnX(columns - 1, 0) + CombatHUDGauge::CellSize : 0.f;
		valueSlot->SetPosition(FVector2D(182 + width + 16, valueSlot->GetPosition().Y));
	}
	PaintHealth(PlayerCells, ViewData.PlayerHealth, true);
	PaintHealth(TargetCells, ViewData.TargetHealth);
	// Bound the visual allocation without changing gameplay's threshold.
	const int32 count = FMath::Clamp(ViewData.BalanceMaximum, 0, 64);
	while (BalanceCells.Num() < count)
	{
		UCPixelAlignedGaugeImage* cell = WidgetTree->ConstructWidget<UCPixelAlignedGaugeImage>();
		cell->bBossOffsetOnly = true;
		cell->SetBrush(*FCoreStyle::Get().GetBrush("WhiteBrush"));
		cell->SetColorAndOpacity(Empty);
		Place(TargetPanel, cell, FVector2D(0, 110), FVector2D(8.5f, 8.5f));
		cell->SetRenderTransformAngle(45.f);
		BalanceCells.Add(cell);
	}
	for (int32 i = 0; i < BalanceCells.Num(); ++i)
	{
		CastChecked<UCanvasPanelSlot>(BalanceCells[i]->Slot)->SetPosition(FVector2D(i * FMath::Min(19.f, 868.f / FMath::Max(1, count)) + 2, 110));
		BalanceCells[i]->SetVisibility(i < count ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		BalanceCells[i]->SetColorAndOpacity(i < ViewData.BalanceRemaining ? FLinearColor(0.82f, 0.69f, 0.13f) : Empty);
	}
}
