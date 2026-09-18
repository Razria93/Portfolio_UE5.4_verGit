#include "UI/CCombatHUDWidget.h"
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
	void PaintHealth(const TArray<TObjectPtr<UImage>>& Cells, const FHUDResourceViewData& Value)
	{
		const float filled = Value.GetFraction() * Cells.Num();
		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			const float coverage = FMath::Clamp(filled - i, 0.f, 1.f);
			Cells[i]->SetColorAndOpacity(FMath::Lerp(Empty, Ink, coverage));
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

void UCCombatHUDWidget::AddGauge(UCanvasPanel* Parent, FVector2D Position, float Width, int32 Count, TArray<TObjectPtr<UImage>>& Cells)
{
	const float step = Width / Count;
	for (int32 i = 0; i < Count; ++i)
		Cells.Add(AddImage(Parent, Position + FVector2D(i * step, 0), FVector2D(step - 2.f, 10.f), Empty));
}

TSharedRef<SWidget> UCCombatHUDWidget::RebuildWidget()
{
	PlayerCells.Reset();
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
	Place(canvas, TargetPanel, FVector2D(950, 48), FVector2D(660, 100));
	TargetName = AddLabel(TargetPanel, TEXT("TARGET"), FVector2D(0, 0), FVector2D(660, 32), 18);
	TargetName->SetJustification(ETextJustify::Center);
	if (NameFont)
	{
		FSlateFontInfo font(NameFont.Get(), 18);
		font.LetterSpacing = 180;
		TargetName->SetFont(font);
	}
	AddGauge(TargetPanel, FVector2D(0, 38), 660.f, 66, TargetCells);
	TargetHealthUnavailable = AddLabel(TargetPanel, TEXT("\u2014"), FVector2D(667, 28), FVector2D(28, 28), 14);
	TArray<TObjectPtr<UImage>> unused;
	AddGauge(TargetPanel, FVector2D(0, 57), 660.f, 66, unused);
	for (UImage* cell : unused)
	{
		cell->SetBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f, Pending, 1.f));
		cell->SetColorAndOpacity(FLinearColor::White);
	}
	AddLabel(TargetPanel, TEXT("—"), FVector2D(667, 48), FVector2D(28, 28), 14);

	PlayerPanel = WidgetTree->ConstructWidget<UCanvasPanel>();
	Place(canvas, PlayerPanel, FVector2D(64, 1230), FVector2D(520, 138));
	const TCHAR* labels[] = { TEXT("BU"), TEXT("BE"), TEXT("SH"), TEXT("HP") };
	for (int32 row = 0; row < 4; ++row)
	{
		const float y = row * 24.f;
		AddLabel(PlayerPanel, labels[row], FVector2D(92, y), FVector2D(42, 26), 15);
		if (row == 3)
		{
			AddGauge(PlayerPanel, FVector2D(140, y + 7), 360.f, 36, PlayerCells);
			PlayerHealthUnavailable = AddLabel(PlayerPanel, TEXT("—"), FVector2D(505, y), FVector2D(28, 26), 15);
		}
		else
		{
			unused.Reset();
			AddGauge(PlayerPanel, FVector2D(140, y + 7), 200.f, 20, unused);
			for (UImage* cell : unused)
			{
				cell->SetBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent, 0.f, Pending, 1.f));
				cell->SetColorAndOpacity(FLinearColor::White);
			}
			AddLabel(PlayerPanel, TEXT("—"), FVector2D(350, y), FVector2D(28, 26), 15);
		}
	}
	for (int32 i = 0; i < 2; ++i)
	{
		AddSlotFrame(PlayerPanel, FVector2D(0, i * 60.f), 44.f, 5.f);
		UImage* icon = AddImage(PlayerPanel, FVector2D(0, i * 60.f), FVector2D(44, 44), Pending);
		if (ItemIcon) icon->SetBrushFromTexture(ItemIcon);
		AddLabel(PlayerPanel, TEXT("—"), FVector2D(52, i * 60.f + 10), FVector2D(24, 24), 15);
	}
	AddLabel(PlayerPanel, TEXT("TODO"), FVector2D(0, 110), FVector2D(70, 22), 10);
	AddSlotFrame(PlayerPanel, FVector2D(12, -32), 20.f, 10.f);

	SkillsPanel = WidgetTree->ConstructWidget<UCanvasPanel>();
	Place(canvas, SkillsPanel, FVector2D(2256, 1136), FVector2D(240, 232));
	const FVector2D positions[] = { FVector2D(84, 30), FVector2D(0, 100), FVector2D(168, 100), FVector2D(84, 170) };
	for (int32 i = 0; i < 4; ++i)
	{
		AddSlotFrame(SkillsPanel, positions[i], 64.f, 32.f);
		AddSlotFrame(SkillsPanel, positions[i] + FVector2D(62, 48), 12.f, 6.f);
		UImage* icon = AddImage(SkillsPanel, positions[i], FVector2D(64, 64), Pending);
		if (SkillIcons.IsValidIndex(i) && SkillIcons[i]) icon->SetBrushFromTexture(SkillIcons[i]);
	}
	AddLabel(SkillsPanel, TEXT("TODO"), FVector2D(84, 0), FVector2D(72, 24), 10);
	AddSlotFrame(SkillsPanel, FVector2D(106, -30), 20.f, 10.f);
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
	TargetHealthUnavailable->SetVisibility(ViewData.TargetHealth.Availability == EHUDResourceAvailability::Available
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	PlayerHealthUnavailable->SetVisibility(ViewData.PlayerHealth.Availability == EHUDResourceAvailability::Available
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	PaintHealth(PlayerCells, ViewData.PlayerHealth);
	PaintHealth(TargetCells, ViewData.TargetHealth);
	// Bound the visual allocation without changing gameplay's threshold.
	const int32 count = FMath::Clamp(ViewData.BalanceMaximum, 0, 64);
	while (BalanceCells.Num() < count)
	{
		UImage* cell = AddImage(TargetPanel, FVector2D(0, 80), FVector2D(6, 6), Empty);
		cell->SetRenderTransformAngle(45.f);
		BalanceCells.Add(cell);
	}
	for (int32 i = 0; i < BalanceCells.Num(); ++i)
	{
		CastChecked<UCanvasPanelSlot>(BalanceCells[i]->Slot)->SetPosition(FVector2D(i * FMath::Min(15.f, 650.f / FMath::Max(1, count)) + 2, 80));
		BalanceCells[i]->SetVisibility(i < count ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		BalanceCells[i]->SetColorAndOpacity(i < ViewData.BalanceRemaining ? FLinearColor(0.82f, 0.69f, 0.13f) : Empty);
	}
}
