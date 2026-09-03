#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FDebugOverlaySettingDefinition;
struct FDebugOverlaySettingsCategory;

class SPortfolioDebugOverlayEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPortfolioDebugOverlayEditorWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TMap<FString, TArray<TSharedPtr<FString>>> EnumOptionsByCVar;
	FText LastFocusCommandStatus;

	void InitializeEnumOptions();
	TSharedRef<SWidget> MakeRegisteredSettingsSections();
	TSharedRef<SWidget> MakeRegisteredSettingsSection(const FDebugOverlaySettingsCategory& InCategory);
	TSharedRef<SWidget> MakeRegisteredSettingRow(const FDebugOverlaySettingDefinition& InDefinition);
	TSharedRef<SWidget> MakeBoolSettingRow(const FDebugOverlaySettingDefinition& InDefinition);
	TSharedRef<SWidget> MakeIntSettingRow(const FDebugOverlaySettingDefinition& InDefinition);
	TSharedRef<SWidget> MakeFloatSettingRow(const FDebugOverlaySettingDefinition& InDefinition);
	TSharedRef<SWidget> MakeEnumSettingRow(const FDebugOverlaySettingDefinition& InDefinition);

	TSharedRef<SWidget> MakeTopLevelSection(const FText& InTitle, const FText& InDescription, const TSharedRef<SWidget>& InContent) const;
	TSharedRef<SWidget> MakeFocusActionsSection();
	TSharedRef<SWidget> MakeSectionCard(const FText& InTitle, const TSharedRef<SWidget>& InContent) const;
};
