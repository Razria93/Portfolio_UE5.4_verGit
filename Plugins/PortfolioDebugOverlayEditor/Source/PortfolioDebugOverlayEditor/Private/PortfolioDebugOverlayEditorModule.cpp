#include "PortfolioDebugOverlayEditorModule.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Editor.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"
#include "Modules/ModuleManager.h"
#include "Selection.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FPortfolioDebugOverlayEditorModule"

namespace
{
	static const FName DebugOverlayTabName(TEXT("PortfolioDebugOverlayEditor"));

	static constexpr const TCHAR* DebugOverlayEnabledCVarName = TEXT("Portfolio.DebugOverlay.Enabled");
	static constexpr const TCHAR* DebugOverlayCollectCVarName = TEXT("Portfolio.DebugOverlay.Collect");
	static constexpr const TCHAR* DebugOverlayEventLogFilterCVarName = TEXT("Portfolio.DebugOverlay.EventLogFilter");
	static constexpr const TCHAR* DebugOverlayEventLogLimitCVarName = TEXT("Portfolio.DebugOverlay.EventLogLimit");
	static constexpr const TCHAR* DebugOverlayHideNoiseEventsCVarName = TEXT("Portfolio.DebugOverlay.HideNoiseEvents");
	static constexpr const TCHAR* DebugOverlayHideCollisionWindowEventsCVarName = TEXT("Portfolio.DebugOverlay.HideCollisionWindowEvents");
	static constexpr const TCHAR* DebugOverlayNearestTargetRadiusCVarName = TEXT("Portfolio.DebugOverlay.NearestTargetRadius");
	static constexpr const TCHAR* DebugOverlaySelectNearestTargetCommand = TEXT("DebugOverlaySelectNearestTarget");
	static constexpr const TCHAR* DebugOverlaySelectRecentCombatTargetCommand = TEXT("DebugOverlaySelectRecentCombatTarget");
	static constexpr const TCHAR* DebugOverlayClearTargetCommand = TEXT("DebugOverlayClearTarget");
	static constexpr const TCHAR* DebugOverlaySelectOutlinerTargetCommand = TEXT("DebugOverlaySelectOutlinerTarget");

	IConsoleVariable* FindDebugOverlayCVar(const TCHAR* InName)
	{
		return IConsoleManager::Get().FindConsoleVariable(InName);
	}

	bool GetDebugOverlayBoolCVar(const TCHAR* InName)
	{
		if (const IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			return consoleVariable->GetInt() != 0;
		}

		return false;
	}

	void SetDebugOverlayBoolCVar(const TCHAR* InName, bool bInValue)
	{
		if (IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			consoleVariable->Set(bInValue ? 1 : 0, ECVF_SetByConsole);
		}
	}

	int32 GetDebugOverlayIntCVar(const TCHAR* InName)
	{
		if (const IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			return consoleVariable->GetInt();
		}

		return 0;
	}

	void SetDebugOverlayIntCVar(const TCHAR* InName, int32 InValue)
	{
		if (IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			consoleVariable->Set(InValue, ECVF_SetByConsole);
		}
	}

	float GetDebugOverlayFloatCVar(const TCHAR* InName)
	{
		if (const IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			return consoleVariable->GetFloat();
		}

		return 0.f;
	}

	void SetDebugOverlayFloatCVar(const TCHAR* InName, float InValue)
	{
		if (IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			consoleVariable->Set(InValue, ECVF_SetByConsole);
		}
	}

	FString GetDebugOverlayStringCVar(const TCHAR* InName)
	{
		if (const IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			return consoleVariable->GetString();
		}

		return FString();
	}

	void SetDebugOverlayStringCVar(const TCHAR* InName, const FString& InValue)
	{
		if (IConsoleVariable* consoleVariable = FindDebugOverlayCVar(InName))
		{
			consoleVariable->Set(*InValue, ECVF_SetByConsole);
		}
	}

	bool IsKnownEventLogFilter(const FString& InValue)
	{
		return InValue.Equals(TEXT("All"), ESearchCase::IgnoreCase)
			|| InValue.Equals(TEXT("Execution"), ESearchCase::IgnoreCase)
			|| InValue.Equals(TEXT("Combat"), ESearchCase::IgnoreCase)
			|| InValue.Equals(TEXT("AI"), ESearchCase::IgnoreCase);
	}

	FText GetCVarAvailabilityText(const TCHAR* InName)
	{
		return FindDebugOverlayCVar(InName)
			? FText::GetEmpty()
			: LOCTEXT("UnavailableCVar", "Unavailable");
	}

	UWorld* FindDebugOverlayPIEWorld()
	{
		if (!GEngine) return nullptr;

		for (const FWorldContext& worldContext : GEngine->GetWorldContexts())
		{
			if (worldContext.WorldType != EWorldType::PIE) continue;

			UWorld* world = worldContext.World();
			if (IsValid(world)) return world;
		}

		return nullptr;
	}

	FText ExecuteDebugOverlayTargetCommand(const TCHAR* InCommand, const FText& InSuccessStatus)
	{
		UWorld* world = FindDebugOverlayPIEWorld();
		if (!IsValid(world))
		{
			return LOCTEXT("PIEWorldNotAvailable", "PIE world not available");
		}

		APlayerController* playerController = world->GetFirstPlayerController();
		if (!IsValid(playerController))
		{
			return LOCTEXT("PlayerControllerNotAvailable", "PlayerController not available");
		}

		playerController->ConsoleCommand(InCommand, true);
		return InSuccessStatus;
	}

	AActor* GetSingleSelectedEditorActor()
	{
		if (!GEditor) return nullptr;

		USelection* selectedActors = GEditor->GetSelectedActors();
		if (!IsValid(selectedActors) || selectedActors->Num() <= 0) return nullptr;

		for (FSelectionIterator it(*selectedActors); it; ++it)
		{
			AActor* actor = Cast<AActor>(*it);
			if (IsValid(actor)) return actor;
		}

		return nullptr;
	}

	FText ExecuteDebugOverlayOutlinerTargetCommand()
	{
		AActor* selectedActor = GetSingleSelectedEditorActor();
		if (!IsValid(selectedActor))
		{
			return LOCTEXT("NoEditorActorSelected", "No editor actor selected");
		}

		const FString actorName = selectedActor->GetName();
		const FString command = FString::Printf(TEXT("%s %s"), DebugOverlaySelectOutlinerTargetCommand, *actorName);
		return ExecuteDebugOverlayTargetCommand(
			*command,
			FText::Format(LOCTEXT("SelectOutlinerActorSent", "Last Command: SelectOutlinerTarget | Actor: {0}"), FText::FromString(actorName)));
	}

	class SPortfolioDebugOverlayEditorWidget : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(SPortfolioDebugOverlayEditorWidget) {}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
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
								DebugOverlayEnabledCVarName)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeBoolCVarRow(
								LOCTEXT("CollectLabel", "Collect"),
								LOCTEXT("CollectHelp", "Collect future debug overlay snapshots and events."),
								DebugOverlayCollectCVarName)
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
								DebugOverlayHideNoiseEventsCVarName)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							MakeBoolCVarRow(
								LOCTEXT("HideCollisionWindowEventsLabel", "Hide Collision Window Events"),
								LOCTEXT("HideCollisionWindowEventsHelp", "Hide collision window lifecycle events from the EventLog display."),
								DebugOverlayHideCollisionWindowEventsCVarName)
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
							MakeFocusCommandSection()
						]
					]
				]
			];
		}

	private:
		TArray<TSharedPtr<FString>> EventLogFilterOptions;
		TSharedPtr<FString> SelectedEventLogFilter;
		TSharedPtr<SComboBox<TSharedPtr<FString>>> EventLogFilterComboBox;
		FText LastFocusCommandStatus;

		TSharedRef<SWidget> MakeBoolCVarRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName) const
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
						return FindDebugOverlayCVar(InCVarName) != nullptr;
					})
					.IsChecked_Lambda([InCVarName]()
					{
						return GetDebugOverlayBoolCVar(InCVarName) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([InCVarName](ECheckBoxState InNewState)
					{
						SetDebugOverlayBoolCVar(InCVarName, InNewState == ECheckBoxState::Checked);
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([InCVarName]()
					{
						return GetCVarAvailabilityText(InCVarName);
					})
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				];
		}

		TSharedRef<SWidget> MakeEventLogFilterRow()
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
						return FindDebugOverlayCVar(DebugOverlayEventLogFilterCVarName) != nullptr;
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
							SetDebugOverlayStringCVar(DebugOverlayEventLogFilterCVarName, *InSelection);
						}
					})
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							const FString currentValue = GetDebugOverlayStringCVar(DebugOverlayEventLogFilterCVarName);
							if (!FindDebugOverlayCVar(DebugOverlayEventLogFilterCVarName))
							{
								return LOCTEXT("EventLogFilterUnavailable", "Unavailable");
							}

							return IsKnownEventLogFilter(currentValue)
								? FText::FromString(currentValue)
								: FText::FromString(FString::Printf(TEXT("Unknown (%s)"), *currentValue));
						})
					]
				];
		}

		TSharedRef<SWidget> MakeEventLogLimitRow() const
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
						return FindDebugOverlayCVar(DebugOverlayEventLogLimitCVarName) != nullptr;
					})
					.MinValue(0)
					.MaxValue(32)
					.MinSliderValue(0)
					.MaxSliderValue(32)
					.Value_Lambda([]()
					{
						return FMath::Clamp(GetDebugOverlayIntCVar(DebugOverlayEventLogLimitCVarName), 0, 32);
					})
					.OnValueChanged_Lambda([](int32 InValue)
					{
						SetDebugOverlayIntCVar(DebugOverlayEventLogLimitCVarName, FMath::Clamp(InValue, 0, 32));
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([]()
					{
						return GetCVarAvailabilityText(DebugOverlayEventLogLimitCVarName);
					})
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				];
		}

		TSharedRef<SWidget> MakeRefreshRow()
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					SNew(STextBlock)
					.Text_Lambda([]()
					{
						const bool bHasAllRequiredCVars =
							FindDebugOverlayCVar(DebugOverlayEnabledCVarName)
							&& FindDebugOverlayCVar(DebugOverlayCollectCVarName)
							&& FindDebugOverlayCVar(DebugOverlayEventLogFilterCVarName)
							&& FindDebugOverlayCVar(DebugOverlayEventLogLimitCVarName)
							&& FindDebugOverlayCVar(DebugOverlayNearestTargetRadiusCVarName)
							&& FindDebugOverlayCVar(DebugOverlayHideNoiseEventsCVarName)
							&& FindDebugOverlayCVar(DebugOverlayHideCollisionWindowEventsCVarName);

						return bHasAllRequiredCVars
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

		TSharedRef<SWidget> MakeNearestTargetRadiusRow() const
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
						.Text(LOCTEXT("NearestTargetRadiusLabel", "Nearest Target Radius"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NearestTargetRadiusHelp", "Radius used by Select Nearest Focus and Recent Combat Focus scan."))
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
						return FindDebugOverlayCVar(DebugOverlayNearestTargetRadiusCVarName) != nullptr;
					})
					.MinValue(0.f)
					.MaxValue(20000.f)
					.MinSliderValue(0.f)
					.MaxSliderValue(10000.f)
					.Delta(50.f)
					.Value_Lambda([]()
					{
						return FMath::Clamp(GetDebugOverlayFloatCVar(DebugOverlayNearestTargetRadiusCVarName), 0.f, 20000.f);
					})
					.OnValueChanged_Lambda([](float InValue)
					{
						SetDebugOverlayFloatCVar(DebugOverlayNearestTargetRadiusCVarName, FMath::Clamp(InValue, 0.f, 20000.f));
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([]()
					{
						return GetCVarAvailabilityText(DebugOverlayNearestTargetRadiusCVarName);
					})
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				];
		}

		TSharedRef<SWidget> MakeFocusCommandSection()
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
					MakeNearestTargetRadiusRow()
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
								LastFocusCommandStatus = ExecuteDebugOverlayTargetCommand(
									DebugOverlaySelectNearestTargetCommand,
									LOCTEXT("SelectNearestTargetSent", "Last Command: SelectNearestTarget"));
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
								LastFocusCommandStatus = ExecuteDebugOverlayOutlinerTargetCommand();
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
							LastFocusCommandStatus = ExecuteDebugOverlayTargetCommand(
								DebugOverlaySelectRecentCombatTargetCommand,
								LOCTEXT("SelectRecentCombatTargetSent", "Last Command: SelectRecentCombatTarget"));
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
							LastFocusCommandStatus = ExecuteDebugOverlayTargetCommand(
								DebugOverlayClearTargetCommand,
								LOCTEXT("ClearTargetSent", "Last Command: ClearTarget"));
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

		void RefreshEventLogFilterSelection()
		{
			const FString currentValue = GetDebugOverlayStringCVar(DebugOverlayEventLogFilterCVarName);
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
	};
}

void FPortfolioDebugOverlayEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		DebugOverlayTabName,
		FOnSpawnTab::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::SpawnDebugOverlayTab))
		.SetDisplayName(LOCTEXT("DebugOverlayTabDisplayName", "Debug Overlay"))
		.SetTooltipText(LOCTEXT("DebugOverlayTabTooltip", "Open Portfolio Debug Overlay settings."))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::RegisterMenus));
}

void FPortfolioDebugOverlayEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DebugOverlayTabName);
}

void FPortfolioDebugOverlayEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped ownerScoped(this);

	UToolMenu* menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
	FToolMenuSection& section = menu->FindOrAddSection(TEXT("PortfolioTools"));
	section.AddSubMenu(
		TEXT("PortfolioToolsDebugOverlay"),
		LOCTEXT("PortfolioToolsSubMenuLabel", "Portfolio Tools"),
		LOCTEXT("PortfolioToolsSubMenuTooltip", "Open Portfolio editor tools."),
		FNewToolMenuDelegate::CreateLambda([this](UToolMenu* subMenu)
		{
			FToolMenuSection& subMenuSection = subMenu->FindOrAddSection(TEXT("DebugOverlay"));
			subMenuSection.AddMenuEntry(
				TEXT("OpenPortfolioDebugOverlayPanel"),
				LOCTEXT("OpenDebugOverlayPanelLabel", "Debug Overlay"),
				LOCTEXT("OpenDebugOverlayPanelTooltip", "Open Portfolio Debug Overlay settings panel."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::OpenDebugOverlayPanel)));
		}));

	UToolMenu* toolbarMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User"));
	FToolMenuSection& toolbarSection = toolbarMenu->FindOrAddSection(TEXT("PortfolioDebugOverlay"));
	FToolMenuEntry toolbarEntry = FToolMenuEntry::InitToolBarButton(
		TEXT("OpenPortfolioDebugOverlayPanelToolbar"),
		FUIAction(FExecuteAction::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::OpenDebugOverlayPanel)),
		LOCTEXT("OpenDebugOverlayPanelToolbarLabel", "Debug Overlay"),
		LOCTEXT("OpenDebugOverlayPanelToolbarTooltip", "Open Debug Overlay panel"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Settings")));
	toolbarEntry.StyleNameOverride = TEXT("AssetEditorToolbar");
	toolbarSection.AddEntry(toolbarEntry);
}

void FPortfolioDebugOverlayEditorModule::OpenDebugOverlayPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(DebugOverlayTabName);
}

TSharedRef<SDockTab> FPortfolioDebugOverlayEditorModule::SpawnDebugOverlayTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPortfolioDebugOverlayEditorWidget)
		];
}

IMPLEMENT_MODULE(FPortfolioDebugOverlayEditorModule, PortfolioDebugOverlayEditor)

#undef LOCTEXT_NAMESPACE
