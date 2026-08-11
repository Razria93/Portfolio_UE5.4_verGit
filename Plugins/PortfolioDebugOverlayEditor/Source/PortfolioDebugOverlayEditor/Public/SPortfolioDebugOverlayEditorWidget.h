#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

template <typename OptionType>
class SComboBox;

class SPortfolioDebugOverlayEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPortfolioDebugOverlayEditorWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// ===== State =====

	TArray<TSharedPtr<FString>> EventLogFilterOptions;
	TSharedPtr<FString> SelectedEventLogFilter;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> EventLogFilterComboBox;
	FText LastFocusCommandStatus;

	// ===== Layout =====

	TSharedRef<SWidget> MakeTopLevelSection(const FText& InTitle, const TSharedRef<SWidget>& InContent) const;
	TSharedRef<SWidget> MakeSectionCard(const FText& InTitle, const TSharedRef<SWidget>& InContent) const;

	// ===== Options Sections =====

	TSharedRef<SWidget> MakeOverlayOptionsSection();
	TSharedRef<SWidget> MakeTargetingDebugSection();
	TSharedRef<SWidget> MakeFocusOptionsSection();

	// ===== CVar Rows =====

	TSharedRef<SWidget> MakeBoolCVarRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName) const;
	TSharedRef<SWidget> MakeEventLogFilterRow();
	TSharedRef<SWidget> MakeEventLogLimitRow() const;
	TSharedRef<SWidget> MakeNearestFocusRadiusRow() const;

	// ===== Status / Refresh =====

	TSharedRef<SWidget> MakeRefreshRow();
	void RefreshEventLogFilterSelection();

};
