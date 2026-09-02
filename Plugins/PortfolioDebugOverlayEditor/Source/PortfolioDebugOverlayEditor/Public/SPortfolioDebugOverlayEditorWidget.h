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
	TArray<TSharedPtr<FString>> EventLogScopeOptions;
	TSharedPtr<FString> SelectedEventLogScope;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> EventLogScopeComboBox;
	FText LastFocusCommandStatus;

	// ===== Layout =====

	TSharedRef<SWidget> MakeTopLevelSection(const FText& InTitle, const TSharedRef<SWidget>& InContent) const;
	TSharedRef<SWidget> MakeSectionCard(const FText& InTitle, const TSharedRef<SWidget>& InContent) const;

	// ===== Options Sections =====

	TSharedRef<SWidget> MakeOverlayOptionsSection();
	TSharedRef<SWidget> MakeMainPanelSections();
	TSharedRef<SWidget> MakeWorldSummarySections();
	TSharedRef<SWidget> MakeTargetingDisplayOptionsSection();
	TSharedRef<SWidget> MakeTargetingDebugSection();
	TSharedRef<SWidget> MakeMovementDisplayOptionsSection();
	TSharedRef<SWidget> MakeMovementDebugSection();
	TSharedRef<SWidget> MakeBalanceDisplayOptionsSection();
	TSharedRef<SWidget> MakeBalanceDebugSection();
	TSharedRef<SWidget> MakeCombatTargetFacingDisplayOptionsSection();
	TSharedRef<SWidget> MakeCombatTargetFacingDebugSection();
	TSharedRef<SWidget> MakeExecutionCollaborationDisplayOptionsSection();
	TSharedRef<SWidget> MakeExecutionCollaborationDebugSection();
	TSharedRef<SWidget> MakeCombatParticipationDisplayOptionsSection();
	TSharedRef<SWidget> MakeCombatParticipationDebugSection();
	TSharedRef<SWidget> MakeFocusOptionsSection();
	TSharedRef<SWidget> MakeFocusSearchSettingsCard() const;
	TSharedRef<SWidget> MakeManualFocusSelectionCard();
	TSharedRef<SWidget> MakeRuntimeFocusSourcesCard();
	TSharedRef<SWidget> MakeClearFocusCard();

	// ===== CVar Rows =====

	TSharedRef<SWidget> MakeBoolCVarRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName, TFunction<bool()> InAdditionalEnabledPredicate = TFunction<bool()>()) const;
	TSharedRef<SWidget> MakeMainPanelChildRow(const FText& InLabel, const FText& InHelp, const TCHAR* InCVarName, const TCHAR* InParentCVarName) const;
	TSharedRef<SWidget> MakeEventLogFilterRow();
	TSharedRef<SWidget> MakeEventLogScopeRow();
	TSharedRef<SWidget> MakeEventLogLimitRow() const;
	TSharedRef<SWidget> MakeNearestFocusRadiusRow() const;

	// ===== Status / Refresh =====

	TSharedRef<SWidget> MakeRefreshRow();
	void RefreshEventLogFilterSelection();
	void RefreshEventLogScopeSelection();

};
