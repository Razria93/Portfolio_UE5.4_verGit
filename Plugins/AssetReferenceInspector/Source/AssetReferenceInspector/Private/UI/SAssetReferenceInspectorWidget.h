#pragma once

#include "CoreMinimal.h"
#include "Analysis/AssetReferenceTypes.h"
#include "AssetRegistry/AssetData.h"
#include "Input/Reply.h"
#include "Types/SlateEnums.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"

class SAssetReferenceInspectorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssetReferenceInspectorWidget)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// Button callbacks
	FReply OnPickSelectedAssetClicked();
	FReply OnAnalyzeClicked();
	FReply OnScanUnusedCandidatesClicked();
	FReply OnExportCsvClicked();
	FReply OnDependenciesModeClicked();
	FReply OnReferencersModeClicked();

	// Filter callbacks
	void OnMaxDepthTextCommitted(const FText& InText, ETextCommit::Type CommitType);
	void OnPathFilterTextCommitted(const FText& InText, ETextCommit::Type CommitType);
	void OnClassFilterTextCommitted(const FText& InText, ETextCommit::Type CommitType);
	void OnIncludeEngineContentChanged(ECheckBoxState NewState);
	void OnIncludePluginContentChanged(ECheckBoxState NewState);

	// Tree callbacks
	void OnTreeNodeDoubleClicked(TSharedPtr<FAssetReferenceTreeNode> Item) const;

	// UI text
	FText GetSelectedAssetText() const;
	FText GetCurrentModeText() const;
	FText GetMaxDepthText() const;
	FText GetPathFilterText() const;
	FText GetClassFilterText() const;
	FSlateFontInfo GetSectionHeaderFont() const;
	FSlateFontInfo GetDependenciesModeFont() const;
	FSlateFontInfo GetReferencersModeFont() const;
	ECheckBoxState GetIncludeEngineContentCheckState() const;
	ECheckBoxState GetIncludePluginContentCheckState() const;

	// Analysis
	void BuildRelationTree();
	void BuildRelationChildren(TSharedPtr<FAssetReferenceTreeNode> ParentNode, TArray<FName>& CurrentPath) const;
	void BuildUnusedCandidateTree();
	void GetRelatedPackageNames(FName PackageName, TArray<FName>& OutPackageNames) const;
	TSharedPtr<FAssetReferenceTreeNode> CreateRelationNode(FName PackageName, int32 Depth, FName ParentPackageName = NAME_None, bool bIsCircular = false) const;
	TSharedPtr<FAssetReferenceTreeNode> CreateUnusedCandidateNode(const FAssetData& AssetData) const;
	FString GetEmptyRelationMessage() const;

	// Asset data
	bool TryGetPrimaryAssetDataForPackage(FName PackageName, FAssetData& OutAssetData) const;
	bool IsUnusedCandidateAsset(FName PackageName, const FAssetData& AssetData) const;
	bool IsExcludedUnusedCandidateClass(const FAssetData& AssetData) const;
	bool TryGetPackageFilename(FName PackageName, FString& OutPackageFilename) const;
	int64 GetPackageDiskSizeBytes(FName PackageName) const;
	int64 GetExistingFileSizeBytes(const FString& Filename) const;

	// Tree view
	void RefreshTree();
	void ExpandTreeItems(const TArray<TSharedPtr<FAssetReferenceTreeNode>>& Items);
	FString FormatSizeBytes(int64 SizeBytes) const;
	FString GetTreeNodeDisplayText(TSharedPtr<FAssetReferenceTreeNode> Item) const;
	TSharedRef<ITableRow> OnGenerateTreeRow(TSharedPtr<FAssetReferenceTreeNode> Item, const TSharedRef<STableViewBase>& OwnerTable) const;
	void OnGetTreeChildren(TSharedPtr<FAssetReferenceTreeNode> Item, TArray<TSharedPtr<FAssetReferenceTreeNode>>& OutChildren) const;

	// Content Browser
	bool TrySyncContentBrowserToPackage(FName PackageName) const;

private:
	// Analysis state
	FAssetReferenceAnalysisOptions AnalysisOptions;
	EAssetReferenceMode CurrentTreeMode = EAssetReferenceMode::Dependencies;
	FAssetData SelectedAssetData;

	// Tree view state
	TArray<TSharedPtr<FAssetReferenceTreeNode>> TreeRootItems;
	TSharedPtr<STreeView<TSharedPtr<FAssetReferenceTreeNode>>> TreeView;
};
