#include "SPortfolioDebugOverlayEditorWidget.h"

#include "FPortfolioDebugOverlayEditorCVarAccess.h"
#include "FPortfolioDebugOverlayEditorFocusCommandBridge.h"

#include "Core/Debug/FDebugOverlaySettingsRegistry.h"

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

void SPortfolioDebugOverlayEditorWidget::Construct(const FArguments& InArgs)
{
	InitializeEnumOptions();
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
					MakeRegisteredSettingsSections()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 12.f)
				[
					MakeFocusActionsSection()
				]
			]
		]
	];
}

void SPortfolioDebugOverlayEditorWidget::InitializeEnumOptions()
{
	EnumOptionsByCVar.Empty();

	for (const FDebugOverlaySettingDefinition& definition : FDebugOverlaySettingsRegistry::GetSettings())
	{
		if (definition.Type != EDebugOverlaySettingType::Enum) continue;

		TArray<TSharedPtr<FString>>& options = EnumOptionsByCVar.FindOrAdd(definition.CVarName);
		for (const FDebugOverlayEnumOption& option : definition.EnumOptions)
		{
			options.Add(MakeShared<FString>(option.DisplayName));
		}
	}
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeRegisteredSettingsSections()
{
	TSharedRef<SVerticalBox> settingsBox = SNew(SVerticalBox);

	for (const FDebugOverlaySettingsCategory& category : FDebugOverlaySettingsRegistry::GetCategories())
	{
		settingsBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 12.f)
		[
			MakeRegisteredSettingsSection(category)
		];
	}

	return settingsBox;
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeRegisteredSettingsSection(const FDebugOverlaySettingsCategory& InCategory)
{
	TSharedRef<SVerticalBox> settingsBox = SNew(SVerticalBox);
	bool bHasSettings = false;

	for (const FDebugOverlaySettingDefinition& definition : FDebugOverlaySettingsRegistry::GetSettings())
	{
		if (definition.Category != InCategory.Id) continue;

		bHasSettings = true;
		settingsBox->AddSlot()
		.AutoHeight()
		.Padding(definition.ParentGateCVarName.IsEmpty() ? 0.f : 16.f, 0.f, 0.f, 0.f)
		[
			MakeRegisteredSettingRow(definition)
		];
	}

	if (!bHasSettings)
	{
		settingsBox->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoRegisteredSettings", "No registered settings."))
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
	}

	return MakeTopLevelSection(
		FText::FromString(InCategory.DisplayName),
		FText::FromString(InCategory.Description),
		settingsBox);
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeRegisteredSettingRow(const FDebugOverlaySettingDefinition& InDefinition)
{
	switch (InDefinition.Type)
	{
	case EDebugOverlaySettingType::Bool:
		return MakeBoolSettingRow(InDefinition);

	case EDebugOverlaySettingType::Int:
		return MakeIntSettingRow(InDefinition);

	case EDebugOverlaySettingType::Float:
		return MakeFloatSettingRow(InDefinition);

	case EDebugOverlaySettingType::Enum:
		return MakeEnumSettingRow(InDefinition);

	default:
		return SNew(STextBlock).Text(LOCTEXT("UnsupportedSettingType", "Unsupported setting type."));
	}
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeBoolSettingRow(const FDebugOverlaySettingDefinition& InDefinition)
{
	const FString cvarName = InDefinition.CVarName;
	const FString parentGate = InDefinition.ParentGateCVarName;
	const FText label = FText::FromString(InDefinition.DisplayName);
	const FText help = FText::FromString(InDefinition.HelpText);

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(label)]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(help)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(12.f, 0.f)
		[
			SNew(SCheckBox)
			.IsEnabled_Lambda([cvarName, parentGate]()
			{
				return CVarAccess::FindCVar(*cvarName)
					&& (parentGate.IsEmpty() || CVarAccess::GetBool(*parentGate));
			})
			.IsChecked_Lambda([cvarName]()
			{
				return CVarAccess::GetBool(*cvarName) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([cvarName](const ECheckBoxState InNewState)
			{
				CVarAccess::SetBool(*cvarName, InNewState == ECheckBoxState::Checked);
			})
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([cvarName]() { return CVarAccess::GetAvailabilityText(*cvarName); })
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeIntSettingRow(const FDebugOverlaySettingDefinition& InDefinition)
{
	const FString cvarName = InDefinition.CVarName;
	const FString parentGate = InDefinition.ParentGateCVarName;
	const FText label = FText::FromString(InDefinition.DisplayName);
	const FText help = FText::FromString(InDefinition.HelpText);
	const int32 minValue = FMath::RoundToInt(InDefinition.MinValue);
	const int32 maxValue = FMath::RoundToInt(InDefinition.MaxValue);

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(label)]
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(help).ColorAndOpacity(FSlateColor::UseSubduedForeground())]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f)
		[
			SNew(SSpinBox<int32>)
			.IsEnabled_Lambda([cvarName, parentGate]()
			{
				return CVarAccess::FindCVar(*cvarName)
					&& (parentGate.IsEmpty() || CVarAccess::GetBool(*parentGate));
			})
			.MinValue(minValue)
			.MaxValue(maxValue)
			.MinSliderValue(minValue)
			.MaxSliderValue(maxValue)
			.Value_Lambda([cvarName, minValue, maxValue]()
			{
				return FMath::Clamp(CVarAccess::GetInt(*cvarName), minValue, maxValue);
			})
			.OnValueChanged_Lambda([cvarName, minValue, maxValue](const int32 InValue)
			{
				CVarAccess::SetInt(*cvarName, FMath::Clamp(InValue, minValue, maxValue));
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([cvarName]() { return CVarAccess::GetAvailabilityText(*cvarName); })
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeFloatSettingRow(const FDebugOverlaySettingDefinition& InDefinition)
{
	const FString cvarName = InDefinition.CVarName;
	const FString parentGate = InDefinition.ParentGateCVarName;
	const FText label = FText::FromString(InDefinition.DisplayName);
	const FText help = FText::FromString(InDefinition.HelpText);
	const float minValue = InDefinition.MinValue;
	const float maxValue = InDefinition.MaxValue;

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(label)]
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(help).ColorAndOpacity(FSlateColor::UseSubduedForeground())]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f)
		[
			SNew(SSpinBox<float>)
			.IsEnabled_Lambda([cvarName, parentGate]()
			{
				return CVarAccess::FindCVar(*cvarName)
					&& (parentGate.IsEmpty() || CVarAccess::GetBool(*parentGate));
			})
			.MinValue(minValue)
			.MaxValue(maxValue)
			.MinSliderValue(minValue)
			.MaxSliderValue(maxValue)
			.Delta(50.f)
			.Value_Lambda([cvarName, minValue, maxValue]()
			{
				return FMath::Clamp(CVarAccess::GetFloat(*cvarName), minValue, maxValue);
			})
			.OnValueChanged_Lambda([cvarName, minValue, maxValue](const float InValue)
			{
				CVarAccess::SetFloat(*cvarName, FMath::Clamp(InValue, minValue, maxValue));
			})
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([cvarName]() { return CVarAccess::GetAvailabilityText(*cvarName); })
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeEnumSettingRow(const FDebugOverlaySettingDefinition& InDefinition)
{
	const FString cvarName = InDefinition.CVarName;
	const FString parentGate = InDefinition.ParentGateCVarName;
	const FText label = FText::FromString(InDefinition.DisplayName);
	const FText help = FText::FromString(InDefinition.HelpText);
	const TArray<TSharedPtr<FString>>& options = EnumOptionsByCVar.FindChecked(cvarName);

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0.f, 4.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(label)]
			+ SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(help).ColorAndOpacity(FSlateColor::UseSubduedForeground())]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(12.f, 0.f)
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.IsEnabled_Lambda([cvarName, parentGate]()
			{
				return CVarAccess::FindCVar(*cvarName)
					&& (parentGate.IsEmpty() || CVarAccess::GetBool(*parentGate));
			})
			.OptionsSource(&options)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
			{
				return SNew(STextBlock).Text(FText::FromString(InItem.IsValid() ? *InItem : FString()));
			})
			.OnSelectionChanged_Lambda([cvarName, InDefinition](TSharedPtr<FString> InSelection, ESelectInfo::Type)
			{
				if (!InSelection.IsValid()) return;

				for (const FDebugOverlayEnumOption& option : InDefinition.EnumOptions)
				{
					if (option.DisplayName.Equals(*InSelection, ESearchCase::CaseSensitive))
					{
						CVarAccess::SetString(*cvarName, option.Value);
						return;
					}
				}
			})
			[
				SNew(STextBlock)
				.Text_Lambda([cvarName, InDefinition]()
				{
					const FString currentValue = CVarAccess::GetString(*cvarName);
					if (!CVarAccess::FindCVar(*cvarName)) return LOCTEXT("EnumUnavailable", "Unavailable");

					for (const FDebugOverlayEnumOption& option : InDefinition.EnumOptions)
					{
						if (option.Value.Equals(currentValue, ESearchCase::IgnoreCase))
						{
							return FText::FromString(option.DisplayName);
						}
					}

					return FText::FromString(FString::Printf(TEXT("Unknown (%s)"), *currentValue));
				})
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([cvarName]() { return CVarAccess::GetAvailabilityText(*cvarName); })
			.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeTopLevelSection(const FText& InTitle, const FText& InDescription, const TSharedRef<SWidget>& InContent) const
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock).Text(InTitle).Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
		[
			SNew(STextBlock).Text(InDescription).ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
		+ SVerticalBox::Slot().AutoHeight()
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
			+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(STextBlock).Text(InTitle).Font(FAppStyle::GetFontStyle("BoldFont"))
			]
			+ SVerticalBox::Slot().AutoHeight()[InContent]
		];
}

TSharedRef<SWidget> SPortfolioDebugOverlayEditorWidget::MakeFocusActionsSection()
{
	return MakeTopLevelSection(
		LOCTEXT("FocusActionsTitle", "Focus Actions"),
		LOCTEXT("FocusActionsDescription", "Select or clear the Enemy used by Character Details and Focused Enemy Event Scope."),
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeSectionCard(
				LOCTEXT("ManualSelectionTitle", "Manual Selection"),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("SelectNearestFocusButton", "Select Nearest Focus"))
					.OnClicked_Lambda([this]()
					{
						LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectNearestFocusCommand();
						return FReply::Handled();
					})
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 6.f, 0.f, 0.f)
				[
					SNew(SButton)
					.Text(LOCTEXT("SelectOutlinerFocusButton", "Select Outliner Focus"))
					.OnClicked_Lambda([this]()
					{
						LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectOutlinerFocusCommand();
						return FReply::Handled();
					})
				]
			)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
		[
			MakeSectionCard(
				LOCTEXT("RuntimeSelectionTitle", "Runtime Sources"),
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SButton)
					.Text(LOCTEXT("SelectPlayerTargetFocusButton", "Select Player Target Focus"))
					.OnClicked_Lambda([this]()
					{
						LastFocusCommandStatus = FocusCommandBridge::ExecuteSelectPlayerTargetFocusCommand();
						return FReply::Handled();
					})
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
				]
			)
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeSectionCard(
				LOCTEXT("ClearSelectionTitle", "Clear"),
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
				]
			)
		]);
}

#undef LOCTEXT_NAMESPACE
