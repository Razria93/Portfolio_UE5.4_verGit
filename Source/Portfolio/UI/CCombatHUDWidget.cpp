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
	// Layout
	constexpr float ReferenceWidth = 2560.f;
	constexpr float ReferenceHeight = 1440.f;
	constexpr float PlayerGaugeX = 182.f;
	constexpr float ResourceValueGap = 16.f;

	// Resource Presentation
	constexpr int32 MaxBalanceCells = 64;
	const float PlayerResourceCapacities[] = { 1600.f, 1400.f, 800.f };

	// Common Colors
	const FLinearColor Ink(0.9f, 0.91f, 0.86f, 1.f);
	const FLinearColor Empty(0.12f, 0.17f, 0.19f, 0.65f);
	const FLinearColor Pending(0.30f, 0.36f, 0.39f, 0.55f);

	// Action Colors
	const FLinearColor ActionUnavailable(0.48f, 0.52f, 0.54f, 0.45f);
	const FLinearColor ActionActive(0.4f, 0.95f, 1.f, 1.f);
	const FLinearColor ExecutionReady(1.f, 0.85f, 0.38f, 1.f);
	const FLinearColor ParrySuccess(1.f, 0.9f, 0.4f, 1.f);

	FLinearColor ResolveActionTint(EHUDActionState State, const FLinearColor& ReadyColor = Ink)
	{
		switch (State)
		{
		case EHUDActionState::Unimplemented: return Pending;
		case EHUDActionState::Ready: return ReadyColor;
		case EHUDActionState::Active: return ActionActive;
		default: return ActionUnavailable;
		}
	}

	void Place(UCanvasPanel* Parent, UWidget* Child, FVector2D Position, FVector2D Size)
	{
		UCanvasPanelSlot* slot = Parent->AddChildToCanvas(Child);
		slot->SetPosition(Position);
		slot->SetSize(Size);
	}

	void PaintHealth(const TArray<TObjectPtr<UImage>>& Cells, const FHUDResourceViewData& Value, bool bFixedUnits = false)
	{
		check(Cells.Num() % CombatHUDGauge::HealthRows == 0);

		const int32 columns = Cells.Num() / CombatHUDGauge::HealthRows;
		const float fraction = Value.GetFraction();

		for (int32 i = 0; i < Cells.Num(); ++i)
		{
			const int32 column = i / CombatHUDGauge::HealthRows;

			float coverage = 0.f;
			if (bFixedUnits)
			{
				// Player HP - Fixed Value per Column
				if (Value.Availability == EHUDResourceAvailability::Available)
					coverage = CombatHUDGauge::UnitFill(Value.Current, Value.Maximum, CombatHUDGauge::PlayerHealthPerColumn, column);
			}
			else
			{
				// Enemy HP - Health Ratio across Fixed Columns
				coverage = CombatHUDGauge::ColumnFill(fraction, column, columns);
			}

			UImage* cell = Cells[i].Get();
			UCanvasPanelSlot* slot = CastChecked<UCanvasPanelSlot>(cell->Slot);

			slot->SetSize(FVector2D(CombatHUDGauge::CellSize * coverage, CombatHUDGauge::CellSize));
			cell->SetVisibility(coverage > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}
}

// Construction

UCCombatHUDWidget::UCCombatHUDWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UFont> label(TEXT("/Game/08_UI/CombatHUD/F_HUDLabel"));
	LabelFont = label.Object;

	static ConstructorHelpers::FObjectFinder<UFont> name(TEXT("/Game/08_UI/CombatHUD/F_HUDName"));
	NameFont = name.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> vial(TEXT("/Game/08_UI/CombatHUD/T_HUDVial"));
	ItemIcon = vial.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> rush(TEXT("/Game/08_UI/CombatHUD/T_HUDActionRush"));
	RushIcon = rush.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> guard(TEXT("/Game/08_UI/CombatHUD/T_HUDActionGuard"));
	GuardIcon = guard.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> dodge(TEXT("/Game/08_UI/CombatHUD/T_HUDActionDodge"));
	DodgeIcon = dodge.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> counter(TEXT("/Game/08_UI/CombatHUD/T_HUDActionCounter"));
	CounterIcon = counter.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> execution(TEXT("/Game/08_UI/CombatHUD/T_HUDActionExecution"));
	ExecutionIcon = execution.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> guardBreak(TEXT("/Game/08_UI/CombatHUD/T_HUDActionGuardBreak"));
	GuardBreakIcon = guardBreak.Object;
}

// Widget Lifecycle

TSharedRef<SWidget> UCCombatHUDWidget::RebuildWidget()
{
	ResetWidgetRuntime();
	UCanvasPanel* root = BuildRootLayout();
	// Samples
	const bool showSamples = ShouldShowResourceSamples();

	BuildTargetPanel(root, showSamples);
	BuildPlayerPanel(root, showSamples);
	BuildActionPanel(root);

	UpdatePresentation();

	return Super::RebuildWidget();
}

// View Data Update

void UCCombatHUDWidget::ApplyViewData(const FCombatHUDViewData& Data)
{
	ViewData = Data;

	UpdatePresentation();
}

void UCCombatHUDWidget::ApplyActionViewData(const FHUDActionViewData& Data)
{
	ViewData.Actions = Data;

	UpdateActionPresentation();
}

// Widget Runtime

void UCCombatHUDWidget::ResetWidgetRuntime()
{
	PlayerPanel = nullptr;
	PlayerHealthGrid = nullptr;
	PlayerHealthValue = nullptr;

	TargetPanel = nullptr;
	TargetName = nullptr;
	TargetHealthValue = nullptr;
	TargetBalanceValue = nullptr;

	SkillsPanel = nullptr;
	GuardImage = nullptr;
	DodgeImage = nullptr;
	CounterImage = nullptr;
	ExecutionImage = nullptr;

	PlayerCells.Reset();
	TargetCells.Reset();
	BalanceCells.Reset();

	PlayerHealthColumns = -1;
	LastPlayerMaximum = -1.f;
}

// Widget Construction

// HUD scale container and root canvas.
UCanvasPanel* UCCombatHUDWidget::BuildRootLayout()
{
	UScaleBox* scaleBox = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("HUDScale"));
	scaleBox->SetStretch(EStretch::ScaleToFit);

	USizeBox* sizeBox = WidgetTree->ConstructWidget<USizeBox>();
	sizeBox->SetWidthOverride(ReferenceWidth);
	sizeBox->SetHeightOverride(ReferenceHeight);
	scaleBox->AddChild(sizeBox);

	UCanvasPanel* canvasPanel = WidgetTree->ConstructWidget<UCanvasPanel>();
	sizeBox->AddChild(canvasPanel);

	WidgetTree->RootWidget = scaleBox;

	SetVisibility(ESlateVisibility::HitTestInvisible);

	return canvasPanel;
}

// Top target panel: name, HP, SH placeholder/sample, and Balance value.
// Balance cells are managed by UpdateTargetBalancePresentation.
void UCCombatHUDWidget::BuildTargetPanel(UCanvasPanel* Root, bool bShowSamples)
{
	TargetPanel = WidgetTree->ConstructWidget<UCanvasPanel>();

	Place(Root, TargetPanel, FVector2D(840, 48), FVector2D(CombatHUDGauge::BossWidth, 132));

	TargetName = AddLabel(TargetPanel, TEXT("TARGET"), FVector2D(0, 0), FVector2D(CombatHUDGauge::BossWidth, 36), 21);
	TargetName->SetJustification(ETextJustify::Center);

	if (NameFont)
	{
		FSlateFontInfo font(NameFont.Get(), 21);
		font.LetterSpacing = 180;
		TargetName->SetFont(font);
	}

	// HP
	TargetHealthValue = AddLabel(TargetPanel, TEXT("\u2014"), FVector2D(888, 42), FVector2D(260, 28), 15);
	AddGauge(TargetPanel, FVector2D(0, 47), CombatHUDGauge::BossWidth, CombatHUDGauge::HealthRows, 0, TargetCells, false, CombatHUDGauge::BossHealthColumns, true);

	// SH
	UTextBlock* shieldValue = AddLabel(TargetPanel, TEXT("\u2014"), FVector2D(888, 73), FVector2D(260, 28), 15);

	TArray<TObjectPtr<UImage>> targetShieldCells;
	AddGauge(TargetPanel, FVector2D(0, 82), CombatHUDGauge::BossWidth, 2, CombatHUDGauge::BossUnitColumns, targetShieldCells, !bShowSamples, CombatHUDGauge::BossUnitColumns * CombatHUDGauge::BossGroups, true);

	// Balance
	TargetBalanceValue = AddLabel(TargetPanel, TEXT("\u2014"), FVector2D(888, 99), FVector2D(260, 28), 15);

	// Samples
	if (bShowSamples)
		ApplyTargetShieldSample(targetShieldCells, shieldValue);
}

// Bottom-left player panel: resources, divider, and item slots.
// HP cells are built by RefreshPlayerHealthLayout.
void UCCombatHUDWidget::BuildPlayerPanel(UCanvasPanel* Root, bool bShowSamples)
{
	PlayerPanel = WidgetTree->ConstructWidget<UCanvasPanel>();

	Place(Root, PlayerPanel, FVector2D(64, 1208), FVector2D(1000, 160));

	AddImage(PlayerPanel, FVector2D(108, 0), FVector2D(2.67f, 128), FLinearColor(0.62f, 0.67f, 0.69f, 0.6f));

	const TCHAR* labels[] = { TEXT("BU"), TEXT("BE"), TEXT("SH"), TEXT("HP") };

	// Samples
	if (bShowSamples)
		AddLabel(PlayerPanel, TEXT("SAMPLE: BU / BE / SH"), FVector2D(PlayerGaugeX, -30), FVector2D(320, 22), 10);

	const float firstRowY = CombatHUDGauge::Height(2) / 2.f - 13.f;
	const float lastRowY = 128.f - CombatHUDGauge::Height(CombatHUDGauge::HealthRows) / 2.f - 14.f;

	for (int32 row = 0; row < 4; ++row)
	{
		const bool bIsHealth = row == 3;
		const bool bIsShield = row == 2;

		const float y = FMath::Lerp(firstRowY, lastRowY, row / 3.f);
		const float gaugeCenterY = y + (bIsHealth ? 14.f : 13.f);

		AddLabel(PlayerPanel, labels[row], FVector2D(126, gaugeCenterY - 16.f), FVector2D(48, 30), 17);

		if (bIsHealth)
		{
			PlayerHealthGrid = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("PlayerHealthGrid"));
			Place(PlayerPanel, PlayerHealthGrid,
				FVector2D(PlayerGaugeX, gaugeCenterY - CombatHUDGauge::Height(CombatHUDGauge::HealthRows) / 2.f),
				FVector2D(CombatHUDGauge::PlayerHealthWidth, CombatHUDGauge::Height(CombatHUDGauge::HealthRows)));

			PlayerHealthValue = AddLabel(PlayerPanel, TEXT("—"), FVector2D(778, gaugeCenterY - 15.f), FVector2D(220, 28), 15);
		}
		else
		{
			TArray<TObjectPtr<UImage>> resourceCells;

			const int32 unitColumns = bIsShield ? CombatHUDGauge::ShieldUnitColumns : 2;
			const float valuePerColumn = bIsShield ? CombatHUDGauge::ShieldPerColumn : 2.f * CombatHUDGauge::EnergyPerCell;

			const int32 columns = CombatHUDGauge::ResourceColumns(PlayerResourceCapacities[row], valuePerColumn, 64);
			const float width = CombatHUDGauge::ColumnX(columns - 1, unitColumns) + CombatHUDGauge::CellSize;

			AddGauge(PlayerPanel, FVector2D(PlayerGaugeX, gaugeCenterY - CombatHUDGauge::Height(2) / 2.f), 320.f, 2, unitColumns, resourceCells, !bShowSamples, columns);

			// Samples
			if (bShowSamples)
			{
				UTextBlock* value = AddLabel(PlayerPanel, TEXT(""), FVector2D(PlayerGaugeX + width + ResourceValueGap, gaugeCenterY - 15.f), FVector2D(220, 28), 15);
				ApplyPlayerResourceSample(row, resourceCells, value);
			}
			else
			{
				AddLabel(PlayerPanel, TEXT("—"), FVector2D(512, gaugeCenterY - 17.f), FVector2D(32, 30), 17);
			}
		}
	}

	BuildItemSlots();
}

// Two item slots with icons and quantity placeholders.
void UCCombatHUDWidget::BuildItemSlots()
{
	for (int32 i = 0; i < 2; ++i)
	{
		AddSlotFrame(PlayerPanel, FVector2D(0, i * 72.f), 56.f, 5.f);

		UImage* icon = AddImage(PlayerPanel, FVector2D(0, i * 72.f), FVector2D(56, 56), Pending);
		if (ItemIcon) icon->SetBrushFromTexture(ItemIcon);

		AddLabel(PlayerPanel, TEXT("—"), FVector2D(66, i * 72.f + 14), FVector2D(28, 28), 17);
	}
}

// Bottom-right actions: Guard (top), Dodge (left), Counter (right), Execution (bottom), Rush (above).
// Visuals only; no input or action execution bindings.
void UCCombatHUDWidget::BuildActionPanel(UCanvasPanel* Root)
{
	SkillsPanel = WidgetTree->ConstructWidget<UCanvasPanel>();

	Place(Root, SkillsPanel, FVector2D(2220, 1100), FVector2D(276, 268));

	GuardImage = AddActionSlot(FVector2D(92, 30), GuardIcon.Get());
	DodgeImage = AddActionSlot(FVector2D(0, 112), DodgeIcon.Get());
	CounterImage = AddActionSlot(FVector2D(184, 112), CounterIcon.Get());
	ExecutionImage = AddActionSlot(FVector2D(92, 194), ExecutionIcon.Get());

	AddSlotFrame(SkillsPanel, FVector2D(114, -38), 28.f, 14.f);

	UImage* rushIcon = AddImage(SkillsPanel, FVector2D(114, -38), FVector2D(28, 28), Pending);
	if (RushIcon) rushIcon->SetBrushFromTexture(RushIcon);
}

// Preview Presentation

// Samples
bool UCCombatHUDWidget::ShouldShowResourceSamples() const
{
#if WITH_EDITOR
	return bShowResourceSamples;
#else
	return false;
#endif
}

// Samples
void UCCombatHUDWidget::ApplyTargetShieldSample(const TArray<TObjectPtr<UImage>>& Cells, UTextBlock* Value)
{
	FHUDResourceViewData sample;

	sample.Availability = EHUDResourceAvailability::Available;
	sample.Current = 2700.f;
	sample.Maximum = 4500.f;

	Value->SetText(sample.GetValueText());

	AddLabel(TargetPanel, TEXT("SAMPLE: SH"), FVector2D(888, 128), FVector2D(200, 22), 10);

	for (int32 i = 0; i < Cells.Num(); ++i)
	{
		const float fill = CombatHUDGauge::ColumnFill(sample.GetFraction(), i / 2, Cells.Num() / 2);

		UImage* cell = Cells[i].Get();
		UCanvasPanelSlot* slot = CastChecked<UCanvasPanelSlot>(cell->Slot);

		slot->SetSize(FVector2D(CombatHUDGauge::CellSize * fill, CombatHUDGauge::CellSize));
		cell->SetColorAndOpacity(FLinearColor(0.35f, 0.8f, 0.65f));
		cell->SetVisibility(fill > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

// Samples
void UCCombatHUDWidget::ApplyPlayerResourceSample(int32 ResourceIndex, const TArray<TObjectPtr<UImage>>& Cells, UTextBlock* Value)
{
	const float current[] = { 1200.f, 700.f, 560.f };
	const FLinearColor colors[] = { FLinearColor(0.9f, 0.35f, 0.25f), FLinearColor(0.3f, 0.72f, 0.9f), FLinearColor(0.35f, 0.8f, 0.65f) };

	FHUDResourceViewData sample;

	sample.Availability = EHUDResourceAvailability::Available;
	sample.Current = current[ResourceIndex];
	sample.Maximum = PlayerResourceCapacities[ResourceIndex];

	const bool bIsShield = ResourceIndex == 2;
	for (int32 i = 0; i < Cells.Num(); ++i)
	{
		const float fill = bIsShield
			? CombatHUDGauge::UnitFill(sample.Current, sample.Maximum, CombatHUDGauge::ShieldPerColumn, i / 2)
			: CombatHUDGauge::UnitFill(sample.Current, sample.Maximum, CombatHUDGauge::EnergyPerCell, i);

		FLinearColor color = colors[ResourceIndex];
		if (!bIsShield) color.A = fill;

		UImage* cell = Cells[i].Get();
		UCanvasPanelSlot* slot = CastChecked<UCanvasPanelSlot>(cell->Slot);

		slot->SetSize(FVector2D(CombatHUDGauge::CellSize * (bIsShield ? fill : 1.f), CombatHUDGauge::CellSize));
		cell->SetColorAndOpacity(color);
		cell->SetVisibility(fill > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	Value->SetText(sample.GetValueText());
}

// Presentation Update

void UCCombatHUDWidget::UpdatePresentation()
{
	if (!PlayerPanel || !PlayerHealthGrid || !PlayerHealthValue
		|| !TargetPanel || !TargetName || !TargetHealthValue || !TargetBalanceValue
		|| !SkillsPanel || !GuardImage || !DodgeImage || !CounterImage || !ExecutionImage) return;

	UpdatePanelVisibility();
	UpdatePlayerPresentation();
	UpdateTargetPresentation();
	UpdateActionPresentation();
}

void UCCombatHUDWidget::UpdatePanelVisibility()
{
	PlayerPanel->SetVisibility(ViewData.bHasPlayer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	SkillsPanel->SetVisibility(ViewData.bHasPlayer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	TargetPanel->SetVisibility(ViewData.bHasTarget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCCombatHUDWidget::UpdatePlayerPresentation()
{
	PlayerHealthValue->SetText(ViewData.PlayerHealth.GetValueText());

	RefreshPlayerHealthLayout();

	PaintHealth(PlayerCells, ViewData.PlayerHealth, true);
}

void UCCombatHUDWidget::UpdateTargetPresentation()
{
	TargetName->SetText(ViewData.TargetName);
	TargetHealthValue->SetText(ViewData.TargetHealth.GetValueText());

	PaintHealth(TargetCells, ViewData.TargetHealth);

	UpdateTargetBalancePresentation();
}

void UCCombatHUDWidget::UpdateActionPresentation()
{
	if (GuardImage)
		GuardImage->SetColorAndOpacity(ViewData.Actions.bParrySuccess ? ParrySuccess : ResolveActionTint(ViewData.Actions.Guard));

	if (DodgeImage)
		DodgeImage->SetColorAndOpacity(ResolveActionTint(ViewData.Actions.Dodge));

	if (CounterImage)
		CounterImage->SetColorAndOpacity(ResolveActionTint(EHUDActionState::Unimplemented));

	if (ExecutionImage)
		ExecutionImage->SetColorAndOpacity(ResolveActionTint(ViewData.Actions.Execution, ExecutionReady));
}

// Resource Presentation

void UCCombatHUDWidget::RefreshPlayerHealthLayout()
{
	const bool bHasValidMaximum =
		ViewData.PlayerHealth.Availability == EHUDResourceAvailability::Available
		&& FMath::IsFinite(ViewData.PlayerHealth.Maximum);

	const float maximum = bHasValidMaximum ? FMath::Max(0.f, ViewData.PlayerHealth.Maximum) : 0.f;
	const int32 limit = CombatHUDGauge::ColumnsForWidth(CombatHUDGauge::PlayerHealthWidth, 0);
	const int32 columns = CombatHUDGauge::ResourceColumns(maximum, CombatHUDGauge::PlayerHealthPerColumn, limit);

	if (maximum != LastPlayerMaximum)
	{
		LastPlayerMaximum = maximum;
		if (maximum > limit * CombatHUDGauge::PlayerHealthPerColumn)
			UE_LOG(LogTemp, Warning, TEXT("CombatHUD: HP max %.0f exceeds display cap %.0f; numeric value unchanged."), maximum, limit * CombatHUDGauge::PlayerHealthPerColumn);
	}

	if (columns != PlayerHealthColumns)
	{
		PlayerHealthGrid->ClearChildren();
		PlayerCells.Reset();
		PlayerHealthColumns = columns;

		AddGauge(PlayerHealthGrid, FVector2D::ZeroVector, CombatHUDGauge::PlayerHealthWidth, CombatHUDGauge::HealthRows, 0, PlayerCells, false, columns);

		const float width = columns > 0 ? CombatHUDGauge::ColumnX(columns - 1, 0) + CombatHUDGauge::CellSize : 0.f;

		UCanvasPanelSlot* valueSlot = CastChecked<UCanvasPanelSlot>(PlayerHealthValue->Slot);
		valueSlot->SetPosition(FVector2D(PlayerGaugeX + width + ResourceValueGap, valueSlot->GetPosition().Y));
	}
}

void UCCombatHUDWidget::UpdateTargetBalancePresentation()
{
	TargetBalanceValue->SetText(ViewData.BalanceMaximum > 0
		? FText::Format(NSLOCTEXT("CombatHUD", "BalanceValue", "{0} / {1}"),
			FText::AsNumber(FMath::Clamp(ViewData.BalanceRemaining, 0, ViewData.BalanceMaximum)), FText::AsNumber(ViewData.BalanceMaximum))
		: FText::FromString(TEXT("\u2014")));

	const int32 count = FMath::Clamp(ViewData.BalanceMaximum, 0, MaxBalanceCells);
	while (BalanceCells.Num() < count)
	{
		UCPixelAlignedGaugeImage* cell = WidgetTree->ConstructWidget<UCPixelAlignedGaugeImage>();

		cell->bBossOffsetOnly = true;
		cell->SetBrush(*FCoreStyle::Get().GetBrush("WhiteBrush"));
		cell->SetColorAndOpacity(Empty);
		cell->SetRenderTransformAngle(45.f);

		Place(TargetPanel, cell, FVector2D(0, 110), FVector2D(8.5f, 8.5f));

		BalanceCells.Add(cell);
	}
	const float spacing = FMath::Min(19.f, 868.f / FMath::Max(1, count));
	for (int32 i = 0; i < BalanceCells.Num(); ++i)
	{
		UImage* cell = BalanceCells[i].Get();
		UCanvasPanelSlot* slot = CastChecked<UCanvasPanelSlot>(cell->Slot);

		slot->SetPosition(FVector2D(i * spacing + 2, 110));
		cell->SetColorAndOpacity(i < ViewData.BalanceRemaining ? FLinearColor(0.82f, 0.69f, 0.13f) : Empty);
		cell->SetVisibility(i < count ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

// Widget Construction Helpers

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

void UCCombatHUDWidget::AddSlotFrame(UCanvasPanel* Parent, FVector2D Position, float Size, float Radius)
{
	UImage* frame = AddImage(Parent, Position, FVector2D(Size), FLinearColor::White);
	frame->SetBrush(FSlateRoundedBoxBrush(FLinearColor(0.015f, 0.02f, 0.025f, 0.25f), Radius, FLinearColor(0.62f, 0.67f, 0.69f, 0.6f), 1.f));
}

UImage* UCCombatHUDWidget::AddActionSlot(FVector2D Position, UTexture2D* Texture)
{
	AddSlotFrame(SkillsPanel, Position, 72.f, 36.f);
	AddSlotFrame(SkillsPanel, Position + FVector2D(68, 52), 24.f, 12.f);

	UImage* image = AddImage(SkillsPanel, Position, FVector2D(72, 72), Pending);
	if (Texture) image->SetBrushFromTexture(Texture);

	return image;
}

void UCCombatHUDWidget::AddGauge(UCanvasPanel* Parent, FVector2D Position, float Width, int32 Rows, int32 UnitColumns, TArray<TObjectPtr<UImage>>& Cells, bool bUnimplemented, int32 ExplicitColumns, bool bBoss)
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
			{
				Cells.Add(addCell(Ink));
			}
		}
	}
}
