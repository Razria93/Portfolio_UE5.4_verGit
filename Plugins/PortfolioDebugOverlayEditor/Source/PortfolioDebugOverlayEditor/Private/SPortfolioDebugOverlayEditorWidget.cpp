#include "SPortfolioDebugOverlayEditorWidget.h"

#include "FPortfolioDebugOverlayEditorCVarAccess.h"
#include "FPortfolioDebugOverlayEditorFocusCommandBridge.h"

#include "Math/UnrealMathUtility.h"
#include "Styling/SlateColor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SPortfolioDebugOverlayEditorWidget"

namespace CVarAccess = PortfolioDebugOverlayEditorCVarAccess;
namespace FocusCommandBridge = PortfolioDebugOverlayEditorFocusCommandBridge;

// ===== Construct =====

void SPortfolioDebugOverlayEditorWidget::Construct(const FArguments& InArgs)
{
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("All")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("Execution")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("Combat")));
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("AI")));
	RefreshEventLogFilterSelection();
	LastFocusCommandStatus = LOCTEXT("FocusCommandNotRun", "Last Command: None");

	ChildSlot
	[
		SNew(SBorder)
		.Padding(10.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PanelDescription", "Session-only controls for Portfolio Debug Overlay CVars. Values are not saved to config."))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeBoolCVarRow(
						LOCTEXT("EnabledLabel", "Enabled"),
						LOCTEXT("EnabledHelp", "Draw the debug overlay HUD."),
						CVarAccess::GetEnabledCVarName())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeBoolCVarRow(
						LOCTEXT("CollectLabel", "Collect"),
						LOCTEXT("CollectHelp", "Collect future debug overlay snapshots and events."),
						CVarAccess::GetCollectCVarName())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeEventLogFilterRow()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeEventLogLimitRow()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 8.f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeBoolCVarRow(
						LOCTEXT("HideNoiseEventsLabel", "Hide Noise Events"),
						LOCTEXT("HideNoiseEventsHelp", "Hide reject/ignore noise from the EventLog display."),
						CVarAccess::GetHideNoiseEventsCVarName())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeBoolCVarRow(
						LOCTEXT("HideCollisionWindowEventsLabel", "Hide Collision Window Events"),
						LOCTEXT("HideCollisionWindowEventsHelp", "Hide collision window lifecycle events from the EventLog display."),
						CVarAccess::GetHideCollisionWindowEventsCVarName())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 12.f, 0.f, 0.f)
				[
					MakeRefreshRow()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 12.f, 0.f, 8.f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeTargetingDebugSection()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 12.f, 0.f, 8.f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeFocusCommandSection()
				]
			]
		]
	];
}

// ===== Targeting Debug =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTargetingDebugSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TargetingSectionTitle", "[Targeting]"))
		]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingEnabledLabel", "Enabled"), LOCTEXT("TargetingEnabledHelp", "Enable targeting debug visualization and detail output."), CVarAccess::GetTargetingEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingRangeLabel", "Range Sphere"), LOCTEXT("TargetingRangeHelp", "Draw the maximum targeting range around the viewpoint."), CVarAccess::GetTargetingDrawRangeSphereCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingSphereLabel", "Selected Target Sphere"), LOCTEXT("TargetingSphereHelp", "Draw a sphere around the current player target."), CVarAccess::GetTargetingDrawSelectedTargetSphereCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingLineLabel", "View Line"), LOCTEXT("TargetingLineHelp", "Draw a line from the viewpoint to the current player target."), CVarAccess::GetTargetingDrawViewLineCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingTextLabel", "World Debug Text"), LOCTEXT("TargetingTextHelp", "Draw distance, Dot and score at the current player target."), CVarAccess::GetTargetingDrawDebugTextCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingDetailsLabel", "Overlay Details"), LOCTEXT("TargetingDetailsHelp", "Show targeting score details in the Debug Overlay."), CVarAccess::GetTargetingShowOverlayDetailsCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingLiveSyncLabel", "Live Sync Player Target"), LOCTEXT("TargetingLiveSyncHelp", "Update PlayerTarget Focus continuously; disabling it freezes the last synced target."), CVarAccess::GetTargetingLiveSyncPlayerTargetCVarName())];
}

// ===== CVar Rows =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeBoolCVarRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(InLabel)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(InHelp)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SNew(SCheckBox)
			.IsEnabled_Lambda([InCVarName]()
			{
				return CVarAccess::FindCVar(InCVarName) != nullptr;
			})
			.IsChecked_Lambda([InCVarName]()
			{
				return CVarAccess::GetBool(InCVarName) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([InCVarName](ECheckBoxState InNewState)
			{
				CVarAccess::SetBool(InCVarName, InNewState == ECheckBoxState::Checked);
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([InCVarName]()
			{
				return CVarAccess::GetAvailabilityText(InCVarName);
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeEventLogFilterRow()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogFilterLabel", "EventLog Filter"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogFilterHelp", "Controls the displayed EventLog category."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SAssignNew(EventLogFilterComboBox, SComboBox<TSharedPtr<FString>>)
			.IsEnabled_Lambda([]()
			{
				return CVarAccess::FindCVar(CVarAccess::GetEventLogFilterCVarName()) != nullptr;
			})
			.OptionsSource(&EventLogFilterOptions)
			.InitiallySelectedItem(SelectedEventLogFilter)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
			{
				return SNew(STextBlock)
					.Text(FText::FromString(InItem.IsValid() ? *InItem : FString()));
			})
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> InSelection, ESelectInfo::Type)
			{
				if (InSelection.IsValid())
				{
					SelectedEventLogFilter = InSelection;
					CVarAccess::SetString(CVarAccess::GetEventLogFilterCVarName(), *InSelection);
				}
			})
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					const FString currentValue = CVarAccess::GetString(CVarAccess::GetEventLogFilterCVarName());
					if (!CVarAccess::FindCVar(CVarAccess::GetEventLogFilterCVarName()))
					{
						return LOCTEXT("EventLogFilterUnavailable", "Unavailable");
					}

					return CVarAccess::IsKnownEventLogFilter(currentValue)
						? FText::FromString(currentValue)
						: FText::FromString(FString::Printf(TEXT("Unknown (%s)"), *currentValue));
				})
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeEventLogLimitRow() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogLimitLabel", "EventLog Limit"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EventLogLimitHelp", "Maximum EventLog lines to display. Range: 0-32."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SNew(SSpinBox<int32>)
			.IsEnabled_Lambda([]()
			{
				return CVarAccess::FindCVar(CVarAccess::GetEventLogLimitCVarName()) != nullptr;
			})
			.MinValue(0)
			.MaxValue(32)
			.MinSliderValue(0)
			.MaxSliderValue(32)
			.Value_Lambda([]()
			{
				return FMath::Clamp(CVarAccess::GetInt(CVarAccess::GetEventLogLimitCVarName()), 0, 32);
			})
			.OnValueChanged_Lambda([](int32 InValue)
			{
				CVarAccess::SetInt(CVarAccess::GetEventLogLimitCVarName(), FMath::Clamp(InValue, 0, 32));
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::GetAvailabilityText(CVarAccess::GetEventLogLimitCVarName());
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeNearestFocusRadiusRow() const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NearestFocusRadiusLabel", "Nearest Focus Radius"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NearestFocusRadiusHelp", "Radius used by Select Nearest Focus and Recent Combat Focus scan."))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SNew(SSpinBox<float>)
			.IsEnabled_Lambda([]()
			{
				return CVarAccess::FindCVar(CVarAccess::GetNearestFocusRadiusCVarName()) != nullptr;
			})
			.MinValue(0.f)
			.MaxValue(20000.f)
			.MinSliderValue(0.f)
			.MaxSliderValue(10000.f)
			.Delta(50.f)
			.Value_Lambda([]()
			{
				return FMath::Clamp(CVarAccess::GetFloat(CVarAccess::GetNearestFocusRadiusCVarName()), 0.f, 20000.f);
			})
			.OnValueChanged_Lambda([](float InValue)
			{
				CVarAccess::SetFloat(CVarAccess::GetNearestFocusRadiusCVarName(), FMath::Clamp(InValue, 0.f, 20000.f));
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::GetAvailabilityText(CVarAccess::GetNearestFocusRadiusCVarName());
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

// ===== Status / Refresh =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeRefreshRow()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasAllRequiredCVars()
					? LOCTEXT("AllCVarsAvailable", "Debug Overlay CVars are available.")
					: LOCTEXT("SomeCVarsUnavailable", "Some Debug Overlay CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshButton", "Refresh"))
			.OnClicked_Lambda([this]()
			{
				RefreshEventLogFilterSelection();
				return FReply::Handled();
			})
		];
}

void SPortfolioDebugOverlayEditorWidget::RefreshEventLogFilterSelection()
{
	const FString currentValue = CVarAccess::GetString(CVarAccess::GetEventLogFilterCVarName());
	SelectedEventLogFilter.Reset();

	for (const TSharedPtr<FString>& option : EventLogFilterOptions)
	{
		if (option.IsValid() && option->Equals(currentValue, ESearchCase::IgnoreCase))
		{
			SelectedEventLogFilter = option;
			break;
		}
	}

	if (EventLogFilterComboBox.IsValid())
	{
		EventLogFilterComboBox->SetSelectedItem(SelectedEventLogFilter);
	}
}

// ===== Focus Commands =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeFocusCommandSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FocusSectionLabel", "Focus"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FocusSectionHelp", "Runs existing debug overlay focus console commands during PIE. Check the HUD for the selection result."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			MakeNearestFocusRadiusRow()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 2.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(0.f, 0.f, 4.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SelectNearestFocusButton", "Select Nearest Focus"))
					.OnClicked_Lambda([this]()
					{
						LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectNearestFocusCommand();
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(4.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SelectOutlinerActorButton", "Select Outliner Focus"))
					.OnClicked_Lambda([this]()
					{
						LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectOutlinerFocusCommand();
						return FReply::Handled();
					})
				]
			]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectRecentCombatFocusButton", "Select Recent Combat Focus"))
				.OnClicked_Lambda([this]()
				{
					LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectRecentCombatFocusCommand();
					return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 10.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectPlayerTargetFocusButton", "Select Player Target Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectPlayerTargetFocusCommand();
				return FReply::Handled();
			})
		]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearFocusButton", "Clear Focus"))
				.OnClicked_Lambda([this]()
				{
					LastFocusCommandStatus = FocusCommandBridge::ExecuteClearFocusCommand();
					return FReply::Handled();
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				return LastFocusCommandStatus;
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

#undef LOCTEXT_NAMESPACE
