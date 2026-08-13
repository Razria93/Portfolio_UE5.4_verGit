#include "SPortfolioDebugOverlayEditorWidget.h"

#include "FPortfolioDebugOverlayEditorCVarAccess.h"
#include "FPortfolioDebugOverlayEditorFocusCommandBridge.h"

#include "Math/UnrealMathUtility.h"
#include "Styling/AppStyle.h"
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
	EventLogFilterOptions.Add(MakeShared<FString>(TEXT("Death")));
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
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					MakeTopLevelSection(LOCTEXT("OverlayOptionsTitle", "Overlay Options"), MakeOverlayOptionsSection())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeTargetingDisplayOptionsSection()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeFocusOptionsSection()
				]
			]
		]
	];
}

// ===== Section Layout =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTopLevelSection(const FText& InTitle, const TSharedRef<SWidget>& InContent) const
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(InTitle)
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				InContent
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeSectionCard(const FText& InTitle, const TSharedRef<SWidget>& InContent) const
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(8.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock)
				.Text(InTitle)
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				InContent
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeOverlayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("EnabledLabel", "Enabled"), LOCTEXT("EnabledHelp", "Draw the debug overlay HUD."), CVarAccess::GetEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("CollectLabel", "Collect"), LOCTEXT("CollectHelp", "Collect future debug overlay snapshots and events."), CVarAccess::GetCollectCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeEventLogFilterRow()]
		+ SVerticalBox::Slot().AutoHeight()[MakeEventLogLimitRow()]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("HideNoiseEventsLabel", "Hide Noise Events"), LOCTEXT("HideNoiseEventsHelp", "Hide reject/ignore noise from the EventLog display."), CVarAccess::GetHideNoiseEventsCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("HideCollisionWindowEventsLabel", "Hide Collision Window Events"), LOCTEXT("HideCollisionWindowEventsHelp", "Hide collision window lifecycle events from the EventLog display."), CVarAccess::GetHideCollisionWindowEventsCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("DiagnosticLoggingTitle", "Diagnostic Logging"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("DeathContractAuditLogLabel", "Death Contract Audit Log"), LOCTEXT("DeathContractAuditLogHelp", "Also write Death lifecycle contract violations to the Output Log."), CVarAccess::GetDeathLifecycleAuditCVarName())]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CVarStatusTitle", "CVar Status"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()[MakeRefreshRow()];
}

// ===== Targeting Debug =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTargetingDisplayOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TargetingDisplayOptionsTitle", "Targeting Display Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasTargetingDisplayCVars()
					? LOCTEXT("TargetingCVarsAvailable", "Targeting Display CVars are available.")
					: LOCTEXT("TargetingCVarsUnavailable", "Targeting Display CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(8.f)
			[
				MakeTargetingDebugSection()
			]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTargetingDebugSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingEnabledLabel", "Enabled"), LOCTEXT("TargetingEnabledHelp", "Enable targeting world debug and Overlay details. World debug remains available when the Overlay HUD is disabled."), CVarAccess::GetTargetingEnabledCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingRangeLabel", "Range Sphere"), LOCTEXT("TargetingRangeHelp", "Draw the maximum targeting range around the viewpoint."), CVarAccess::GetTargetingDrawRangeSphereCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingSphereLabel", "Selected Target Sphere"), LOCTEXT("TargetingSphereHelp", "Draw a sphere around the current player target."), CVarAccess::GetTargetingDrawSelectedTargetSphereCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingLineLabel", "View Line"), LOCTEXT("TargetingLineHelp", "Draw a line from the viewpoint to the current player target."), CVarAccess::GetTargetingDrawViewLineCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingTextLabel", "World Debug Text"), LOCTEXT("TargetingTextHelp", "Draw distance, Dot and score at the current player target."), CVarAccess::GetTargetingDrawDebugTextCVarName())]
		+ SVerticalBox::Slot().AutoHeight()[MakeBoolCVarRow(LOCTEXT("TargetingDetailsLabel", "Overlay Details"), LOCTEXT("TargetingDetailsHelp", "Show targeting score details in the Debug Overlay."), CVarAccess::GetTargetingShowOverlayDetailsCVarName())];
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
				.Text(LOCTEXT("NearestFocusRadiusLabel", "Focus Search Radius"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("NearestFocusRadiusHelp", "Search radius used by Nearest and Recent Combat focus selection."))
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
				return CVarAccess::HasOverlayCVars()
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

// ===== Focus Options =====

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeFocusOptionsSection()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FocusOptionsTitle", "Focus Options"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([]()
			{
				return CVarAccess::HasFocusCVars()
					? LOCTEXT("FocusCVarsAvailable", "Focus CVars are available.")
					: LOCTEXT("FocusCVarsUnavailable", "Focus CVars are unavailable. Start the game module or PIE if needed.");
			})
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeFocusSearchSettingsCard()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeManualFocusSelectionCard()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeRuntimeFocusSourcesCard()
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeClearFocusCard()
		]
		;
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeFocusSearchSettingsCard() const
{
	return MakeSectionCard(
		LOCTEXT("FocusSearchSettingsGroupTitle", "Search Settings"),
		MakeNearestFocusRadiusRow());
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeManualFocusSelectionCard()
{
	return MakeSectionCard(
		LOCTEXT("ManualFocusSelectionGroupTitle", "Manual Selection"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NearestFocusSourceLabel", "Nearest"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NearestFocusSourceHelp", "Select the nearest eligible actor around the player."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectNearestFocusButton", "Select Nearest Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectNearestFocusCommand();
				return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OutlinerFocusSourceLabel", "Outliner"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OutlinerFocusSourceHelp", "Use the actor currently selected in the editor Outliner."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectOutlinerActorButton", "Select Outliner Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectOutlinerFocusCommand();
				return FReply::Handled();
			})
		]);
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeRuntimeFocusSourcesCard()
{
	return MakeSectionCard(
		LOCTEXT("RuntimeFocusSourcesGroupTitle", "Runtime Sources"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PlayerTargetFocusSourceLabel", "Player Target"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeBoolCVarRow(
				LOCTEXT("TargetingLiveSyncLabel", "Live Sync Player Target"),
				LOCTEXT("TargetingLiveSyncHelp", "Update PlayerTarget Focus continuously; disabling it freezes the last synced target."),
				CVarAccess::GetFocusLiveSyncPlayerTargetCVarName())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectPlayerTargetFocusButton", "Select Player Target Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectPlayerTargetFocusCommand();
				return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 8.f)[SNew(SSeparator)]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RecentCombatFocusSourceLabel", "Recent Combat"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RecentCombatFocusSourceHelp", "Select the latest eligible actor recorded by the runtime combat context."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(SButton)
			.Text(LOCTEXT("SelectRecentCombatFocusButton", "Select Recent Combat Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectRecentCombatFocusCommand();
				return FReply::Handled();
			})
		]);
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeClearFocusCard()
{
	return MakeSectionCard(
		LOCTEXT("ClearFocusGroupTitle", "Clear"),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SButton)
			.Text(LOCTEXT("ClearFocusButton", "Clear Focus"))
			.OnClicked_Lambda([this]()
			{
				LastFocusCommandStatus = FocusCommandBridge::ExecuteClearFocusCommand();
				return FReply::Handled();
			})
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { return LastFocusCommandStatus; })
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]);
}

#undef LOCTEXT_NAMESPACE
