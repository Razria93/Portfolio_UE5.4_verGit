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

	// ===== CVar Rows =====

	TSharedRef<SWidget> MakeBoolCVarRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName) const;
	TSharedRef<SWidget> MakeEventLogFilterRow();
	TSharedRef<SWidget> MakeEventLogLimitRow() const;
	TSharedRef<SWidget> MakeNearestTargetRadiusRow() const;

	// ===== Status / Refresh =====

	TSharedRef<SWidget> MakeRefreshRow();
	void RefreshEventLogFilterSelection();

	// ===== Focus Commands =====

	TSharedRef<SWidget> MakeFocusCommandSection();
};
