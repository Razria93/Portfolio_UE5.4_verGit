#include "UI/SAssetReferenceInspectorWidget.h"

#include "Analysis/AssetReferenceFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Export/AssetReferenceCsvExporter.h"
#include "Modules/ModuleManager.h"

#include "ContentBrowserModule.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "IContentBrowserSingleton.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	constexpr int32 MaxAllowedRelationDepth = 10;
}

void SAssetReferenceInspectorWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
		[
			SNew(SBorder)
				.Padding(12.0f)
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Asset Reference Inspector")))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 8.0f, 0.0f, 8.0f)
						[
							SNew(SSeparator)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 8.0f)
						[
							SNew(STextBlock)
								.Text(this, &SAssetReferenceInspectorWidget::GetSelectedAssetText)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 8.0f)
						[
							SNew(SUniformGridPanel)
								.SlotPadding(4.0f)

								+ SUniformGridPanel::Slot(0, 0)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Pick Selected Asset")))
										.OnClicked(this, &SAssetReferenceInspectorWidget::OnPickSelectedAssetClicked)
								]

								+ SUniformGridPanel::Slot(1, 0)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Analyze")))
										.OnClicked(this, &SAssetReferenceInspectorWidget::OnAnalyzeClicked)
								]

								+ SUniformGridPanel::Slot(2, 0)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Scan Unused Candidates")))
										.OnClicked(this, &SAssetReferenceInspectorWidget::OnScanUnusedCandidatesClicked)
								]

								+ SUniformGridPanel::Slot(3, 0)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Export CSV")))
										.OnClicked(this, &SAssetReferenceInspectorWidget::OnExportCsvClicked)
								]
						]

					+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 8.0f)
						[
							SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 4.0f)
								[
									SNew(STextBlock)
										.Text(FText::FromString(TEXT("Analysis Options")))
										.Font(this, &SAssetReferenceInspectorWidget::GetSectionHeaderFont)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SBorder)
										.Padding(8.0f)
										[
											SNew(SVerticalBox)

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0.0f, 0.0f, 0.0f, 4.0f)
												[
													SNew(STextBlock)
														.Text(FText::FromString(TEXT("Mode")))
														.Font(this, &SAssetReferenceInspectorWidget::GetSectionHeaderFont)
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0.0f, 0.0f, 0.0f, 4.0f)
												[
													SNew(STextBlock)
														.Text(this, &SAssetReferenceInspectorWidget::GetCurrentModeText)
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SNew(SUniformGridPanel)
														.SlotPadding(4.0f)

														+ SUniformGridPanel::Slot(0, 0)
														[
															SNew(SButton)
																.OnClicked(this, &SAssetReferenceInspectorWidget::OnDependenciesModeClicked)
																[
																	SNew(STextBlock)
																		.Text(FText::FromString(TEXT("Dependencies")))
																		.Font(this, &SAssetReferenceInspectorWidget::GetDependenciesModeFont)
																]
														]

													+ SUniformGridPanel::Slot(1, 0)
														[
															SNew(SButton)
																.OnClicked(this, &SAssetReferenceInspectorWidget::OnReferencersModeClicked)
																[
																	SNew(STextBlock)
																		.Text(FText::FromString(TEXT("Referencers")))
																		.Font(this, &SAssetReferenceInspectorWidget::GetReferencersModeFont)
																]
														]
												]
										]
								]

							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SBorder)
										.Padding(8.0f)
										[
											SNew(SVerticalBox)

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0.0f, 0.0f, 0.0f, 6.0f)
												[
													SNew(STextBlock)
														.Text(FText::FromString(TEXT("Filters")))
														.Font(this, &SAssetReferenceInspectorWidget::GetSectionHeaderFont)
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SNew(SGridPanel)
														.FillColumn(1, 1.0f)

														+ SGridPanel::Slot(0, 0)
														.Padding(0.0f, 0.0f, 8.0f, 6.0f)
														[
															SNew(STextBlock)
																.Text(FText::FromString(TEXT("Max Depth")))
														]

														+ SGridPanel::Slot(1, 0)
														.Padding(0.0f, 0.0f, 0.0f, 6.0f)
														[
															SNew(SEditableTextBox)
																.Text(this, &SAssetReferenceInspectorWidget::GetMaxDepthText)
																.OnTextCommitted(this, &SAssetReferenceInspectorWidget::OnMaxDepthTextCommitted)
														]

														+ SGridPanel::Slot(0, 1)
														.Padding(0.0f, 0.0f, 8.0f, 6.0f)
														[
															SNew(STextBlock)
																.Text(FText::FromString(TEXT("Path Filter")))
														]

														+ SGridPanel::Slot(1, 1)
														.Padding(0.0f, 0.0f, 0.0f, 6.0f)
														[
															SNew(SEditableTextBox)
																.Text(this, &SAssetReferenceInspectorWidget::GetPathFilterText)
																.OnTextCommitted(this, &SAssetReferenceInspectorWidget::OnPathFilterTextCommitted)
														]

														+ SGridPanel::Slot(0, 2)
														.Padding(0.0f, 0.0f, 8.0f, 0.0f)
														[
															SNew(STextBlock)
																.Text(FText::FromString(TEXT("Class Filter")))
														]

														+ SGridPanel::Slot(1, 2)
														[
															SNew(SEditableTextBox)
																.Text(this, &SAssetReferenceInspectorWidget::GetClassFilterText)
																.OnTextCommitted(this, &SAssetReferenceInspectorWidget::OnClassFilterTextCommitted)
														]
												]
										]
								]

							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f)
								[
									SNew(SBorder)
										.Padding(8.0f)
										[
											SNew(SVerticalBox)

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(0.0f, 0.0f, 0.0f, 6.0f)
												[
													SNew(STextBlock)
														.Text(FText::FromString(TEXT("Content Scope")))
														.Font(this, &SAssetReferenceInspectorWidget::GetSectionHeaderFont)
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												[
													SNew(SUniformGridPanel)
														.SlotPadding(4.0f)

														+ SUniformGridPanel::Slot(0, 0)
														[
															SNew(SCheckBox)
																.IsChecked(this, &SAssetReferenceInspectorWidget::GetIncludeEngineContentCheckState)
																.OnCheckStateChanged(this, &SAssetReferenceInspectorWidget::OnIncludeEngineContentChanged)
																[
																	SNew(STextBlock)
																		.Text(FText::FromString(TEXT("Engine Content")))
																]
														]

													+ SUniformGridPanel::Slot(1, 0)
														[
															SNew(SCheckBox)
																.IsChecked(this, &SAssetReferenceInspectorWidget::GetIncludePluginContentCheckState)
																.OnCheckStateChanged(this, &SAssetReferenceInspectorWidget::OnIncludePluginContentChanged)
																[
																	SNew(STextBlock)
																		.Text(FText::FromString(TEXT("Plugin Content")))
																]
														]
												]
										]
								]
						]

					+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SNew(SBorder)
								.Padding(8.0f)
								[
									SAssignNew(TreeView, STreeView<TSharedPtr<FAssetReferenceTreeNode>>)
										.TreeItemsSource(&TreeRootItems)
										.OnGenerateRow(this, &SAssetReferenceInspectorWidget::OnGenerateTreeRow)
										.OnGetChildren(this, &SAssetReferenceInspectorWidget::OnGetTreeChildren)
										.OnMouseButtonDoubleClick(this, &SAssetReferenceInspectorWidget::OnTreeNodeDoubleClicked)
								]
						]
				]
		];

	if (TreeView.IsValid())
	{
		for (const TSharedPtr<FAssetReferenceTreeNode>& RootItem : TreeRootItems)
		{
			TreeView->SetItemExpansion(RootItem, true);
		}
	}
}

FReply SAssetReferenceInspectorWidget::OnPickSelectedAssetClicked()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	SelectedAssetData = SelectedAssets.Num() > 0 ? SelectedAssets[0] : FAssetData();

	return FReply::Handled();
}

FReply SAssetReferenceInspectorWidget::OnAnalyzeClicked()
{
	BuildRelationTree();
	RefreshTree();

	return FReply::Handled();
}

FReply SAssetReferenceInspectorWidget::OnScanUnusedCandidatesClicked()
{
	BuildUnusedCandidateTree();
	RefreshTree();

	return FReply::Handled();
}

FReply SAssetReferenceInspectorWidget::OnExportCsvClicked()
{
	FString ExportFilename;
	FString ErrorMessage;
	const bool bExported = FAssetReferenceCsvExporter::ExportToTimestampedCsv(TreeRootItems, CurrentTreeMode, ExportFilename, ErrorMessage);

	FNotificationInfo NotificationInfo(FText::FromString(bExported
		? FString::Printf(TEXT("CSV exported: %s"), *ExportFilename)
		: ErrorMessage));
	NotificationInfo.ExpireDuration = bExported ? 5.0f : 8.0f;

	if (TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(NotificationInfo))
	{
		Notification->SetCompletionState(bExported
			? SNotificationItem::CS_Success
			: SNotificationItem::CS_Fail);
	}

	return FReply::Handled();
}

FReply SAssetReferenceInspectorWidget::OnDependenciesModeClicked()
{
	AnalysisOptions.Mode = EAssetReferenceMode::Dependencies;
	return FReply::Handled();
}

FReply SAssetReferenceInspectorWidget::OnReferencersModeClicked()
{
	AnalysisOptions.Mode = EAssetReferenceMode::Referencers;
	return FReply::Handled();
}

void SAssetReferenceInspectorWidget::OnMaxDepthTextCommitted(const FText& InText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnCleared)
	{
		return;
	}

	int32 ParsedMaxDepth = AnalysisOptions.MaxDepth;
	if (FDefaultValueHelper::ParseInt(InText.ToString().TrimStartAndEnd(), ParsedMaxDepth))
	{
		AnalysisOptions.MaxDepth = FMath::Clamp(ParsedMaxDepth, 0, MaxAllowedRelationDepth);
	}
}

void SAssetReferenceInspectorWidget::OnPathFilterTextCommitted(const FText& InText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnCleared)
	{
		return;
	}

	AnalysisOptions.PathFilter = InText.ToString().TrimStartAndEnd();
}

void SAssetReferenceInspectorWidget::OnClassFilterTextCommitted(const FText& InText, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnCleared)
	{
		return;
	}

	AnalysisOptions.ClassFilter = InText.ToString().TrimStartAndEnd();
}

void SAssetReferenceInspectorWidget::OnIncludeEngineContentChanged(ECheckBoxState NewState)
{
	AnalysisOptions.bIncludeEngineContent = NewState == ECheckBoxState::Checked;
}

void SAssetReferenceInspectorWidget::OnIncludePluginContentChanged(ECheckBoxState NewState)
{
	AnalysisOptions.bIncludePluginContent = NewState == ECheckBoxState::Checked;
}

void SAssetReferenceInspectorWidget::OnTreeNodeDoubleClicked(TSharedPtr<FAssetReferenceTreeNode> Item) const
{
	if (!Item.IsValid())
	{
		return;
	}

	TrySyncContentBrowserToPackage(Item->PackageName);
}

FText SAssetReferenceInspectorWidget::GetSelectedAssetText() const
{
	if (!SelectedAssetData.IsValid())
	{
		return FText::FromString(TEXT("Selected Asset: None"));
	}

	return FText::FromString(FString::Printf(
		TEXT("Selected Asset: %s (%s)"),
		*SelectedAssetData.AssetName.ToString(),
		*SelectedAssetData.PackageName.ToString()));
}

FText SAssetReferenceInspectorWidget::GetCurrentModeText() const
{
	const FString ModeName = AnalysisOptions.Mode == EAssetReferenceMode::Referencers
		? FString(TEXT("Referencers"))
		: FString(TEXT("Dependencies"));

	return FText::FromString(FString::Printf(TEXT("Mode: %s"), *ModeName));
}

FText SAssetReferenceInspectorWidget::GetMaxDepthText() const
{
	return FText::AsNumber(AnalysisOptions.MaxDepth);
}

FText SAssetReferenceInspectorWidget::GetPathFilterText() const
{
	return FText::FromString(AnalysisOptions.PathFilter);
}

FText SAssetReferenceInspectorWidget::GetClassFilterText() const
{
	return FText::FromString(AnalysisOptions.ClassFilter);
}

FSlateFontInfo SAssetReferenceInspectorWidget::GetSectionHeaderFont() const
{
	return FAppStyle::GetFontStyle(TEXT("NormalFontBold"));
}

FSlateFontInfo SAssetReferenceInspectorWidget::GetDependenciesModeFont() const
{
	return AnalysisOptions.Mode == EAssetReferenceMode::Dependencies
		? FAppStyle::GetFontStyle(TEXT("NormalFontBold"))
		: FAppStyle::GetFontStyle(TEXT("NormalFont"));
}

FSlateFontInfo SAssetReferenceInspectorWidget::GetReferencersModeFont() const
{
	return AnalysisOptions.Mode == EAssetReferenceMode::Referencers
		? FAppStyle::GetFontStyle(TEXT("NormalFontBold"))
		: FAppStyle::GetFontStyle(TEXT("NormalFont"));
}

ECheckBoxState SAssetReferenceInspectorWidget::GetIncludeEngineContentCheckState() const
{
	return AnalysisOptions.bIncludeEngineContent
		? ECheckBoxState::Checked
		: ECheckBoxState::Unchecked;
}

ECheckBoxState SAssetReferenceInspectorWidget::GetIncludePluginContentCheckState() const
{
	return AnalysisOptions.bIncludePluginContent
		? ECheckBoxState::Checked
		: ECheckBoxState::Unchecked;
}

void SAssetReferenceInspectorWidget::BuildRelationTree()
{
	TreeRootItems.Reset();
	CurrentTreeMode = AnalysisOptions.Mode;

	if (!SelectedAssetData.IsValid())
	{
		TreeRootItems.Add(MakeShared<FAssetReferenceTreeNode>(TEXT("No selected asset")));
		return;
	}

	TSharedPtr<FAssetReferenceTreeNode> RootNode = MakeShared<FAssetReferenceTreeNode>(
		SelectedAssetData.AssetName.ToString(),
		SelectedAssetData.PackageName,
		0,
		false,
		SelectedAssetData.AssetClassPath.GetAssetName().ToString(),
		GetPackageDiskSizeBytes(SelectedAssetData.PackageName));

	TArray<FName> CurrentPath;
	CurrentPath.Add(SelectedAssetData.PackageName);

	BuildRelationChildren(RootNode, CurrentPath);

	TreeRootItems.Add(RootNode);
}

void SAssetReferenceInspectorWidget::BuildRelationChildren(TSharedPtr<FAssetReferenceTreeNode> ParentNode, TArray<FName>& CurrentPath) const
{
	if (!ParentNode.IsValid() || ParentNode->Depth >= AnalysisOptions.MaxDepth)
	{
		return;
	}

	TArray<FName> RelatedPackageNames;
	GetRelatedPackageNames(ParentNode->PackageName, RelatedPackageNames);

	bool bAddedChild = false;

	for (const FName RelatedPackageName : RelatedPackageNames)
	{
		if (!FAssetReferenceFilter::ShouldPassRelationFilters(AnalysisOptions, RelatedPackageName))
		{
			continue;
		}

		const bool bIsCircular = CurrentPath.Contains(RelatedPackageName);
		TSharedPtr<FAssetReferenceTreeNode> ChildNode = CreateRelationNode(RelatedPackageName, ParentNode->Depth + 1, ParentNode->PackageName, bIsCircular);
		ParentNode->Children.Add(ChildNode);
		bAddedChild = true;

		if (bIsCircular)
		{
			continue;
		}

		CurrentPath.Add(RelatedPackageName);
		BuildRelationChildren(ChildNode, CurrentPath);
		CurrentPath.Pop();
	}

	if (!bAddedChild)
	{
		ParentNode->Children.Add(MakeShared<FAssetReferenceTreeNode>(GetEmptyRelationMessage(), NAME_None, ParentNode->Depth + 1));
	}
}

void SAssetReferenceInspectorWidget::GetRelatedPackageNames(FName PackageName, TArray<FName>& OutPackageNames) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	if (AnalysisOptions.Mode == EAssetReferenceMode::Referencers)
	{
		AssetRegistryModule.Get().GetReferencers(PackageName, OutPackageNames);
		return;
	}

	AssetRegistryModule.Get().GetDependencies(PackageName, OutPackageNames);
}

void SAssetReferenceInspectorWidget::BuildUnusedCandidateTree()
{
	TreeRootItems.Reset();
	CurrentTreeMode = AnalysisOptions.Mode;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().SearchAllAssets(true);
	AssetRegistryModule.Get().WaitForCompletion();

	TArray<FAssetData> ProjectAssets;
	AssetRegistryModule.Get().GetAssetsByPath(FName(TEXT("/Game")), ProjectAssets, true);

	TSharedPtr<FAssetReferenceTreeNode> RootNode = MakeShared<FAssetReferenceTreeNode>(TEXT("Unused Candidates"));

	for (const FAssetData& AssetData : ProjectAssets)
	{
		if (!AssetData.IsValid())
		{
			continue;
		}

		if (!FAssetReferenceFilter::ShouldPassRelationFilters(AnalysisOptions, AssetData.PackageName))
		{
			continue;
		}

		if (!IsUnusedCandidateAsset(AssetData.PackageName, AssetData))
		{
			continue;
		}

		RootNode->Children.Add(CreateUnusedCandidateNode(AssetData));
	}

	RootNode->Children.Sort([](const TSharedPtr<FAssetReferenceTreeNode>& Left, const TSharedPtr<FAssetReferenceTreeNode>& Right)
		{
			const FString LeftName = Left.IsValid() ? Left->DisplayName : FString();
			const FString RightName = Right.IsValid() ? Right->DisplayName : FString();
			return LeftName < RightName;
		});

	if (RootNode->Children.Num() == 0)
	{
		RootNode->Children.Add(MakeShared<FAssetReferenceTreeNode>(TEXT("No unused candidates found"), NAME_None, 1));
	}

	TreeRootItems.Add(RootNode);
}

void SAssetReferenceInspectorWidget::RefreshTree()
{
	if (!TreeView.IsValid())
	{
		return;
	}

	TreeView->RequestTreeRefresh();

	ExpandTreeItems(TreeRootItems);
}

void SAssetReferenceInspectorWidget::ExpandTreeItems(const TArray<TSharedPtr<FAssetReferenceTreeNode>>& Items)
{
	if (!TreeView.IsValid())
	{
		return;
	}

	for (const TSharedPtr<FAssetReferenceTreeNode>& Item : Items)
	{
		if (Item.IsValid())
		{
			TreeView->SetItemExpansion(Item, true);
			ExpandTreeItems(Item->Children);
		}
	}
}

TSharedPtr<FAssetReferenceTreeNode> SAssetReferenceInspectorWidget::CreateRelationNode(FName PackageName, int32 Depth, FName ParentPackageName, bool bIsCircular) const
{
	FString DisplayName = PackageName.ToString();
	FString ClassName;

	FAssetData AssetData;
	if (TryGetPrimaryAssetDataForPackage(PackageName, AssetData))
	{
		DisplayName = AssetData.AssetName.ToString();
		ClassName = AssetData.AssetClassPath.GetAssetName().ToString();
	}

	return MakeShared<FAssetReferenceTreeNode>(
		DisplayName,
		PackageName,
		Depth,
		bIsCircular,
		ClassName,
		GetPackageDiskSizeBytes(PackageName),
		false,
		ParentPackageName);
}

TSharedPtr<FAssetReferenceTreeNode> SAssetReferenceInspectorWidget::CreateUnusedCandidateNode(const FAssetData& AssetData) const
{
	return MakeShared<FAssetReferenceTreeNode>(
		AssetData.AssetName.ToString(),
		AssetData.PackageName,
		1,
		false,
		AssetData.AssetClassPath.GetAssetName().ToString(),
		GetPackageDiskSizeBytes(AssetData.PackageName),
		true);
}

FString SAssetReferenceInspectorWidget::GetEmptyRelationMessage() const
{
	return AnalysisOptions.Mode == EAssetReferenceMode::Referencers
		? FString(TEXT("No referencers found"))
		: FString(TEXT("No dependencies found"));
}

bool SAssetReferenceInspectorWidget::TryGetPrimaryAssetDataForPackage(FName PackageName, FAssetData& OutAssetData) const
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FAssetData> AssetsInPackage;
	AssetRegistryModule.Get().GetAssetsByPackageName(PackageName, AssetsInPackage);

	if (AssetsInPackage.Num() == 0 || !AssetsInPackage[0].IsValid())
	{
		return false;
	}

	OutAssetData = AssetsInPackage[0];
	return true;
}

bool SAssetReferenceInspectorWidget::IsUnusedCandidateAsset(FName PackageName, const FAssetData& AssetData) const
{
	if (PackageName.IsNone() || !AssetData.IsValid())
	{
		return false;
	}

	if (!PackageName.ToString().StartsWith(TEXT("/Game/")))
	{
		return false;
	}

	if (IsExcludedUnusedCandidateClass(AssetData))
	{
		return false;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FName> ReferencerPackageNames;
	AssetRegistryModule.Get().GetReferencers(PackageName, ReferencerPackageNames);

	return ReferencerPackageNames.Num() == 0;
}

bool SAssetReferenceInspectorWidget::IsExcludedUnusedCandidateClass(const FAssetData& AssetData) const
{
	const FString ClassName = AssetData.AssetClassPath.GetAssetName().ToString();

	return ClassName.Equals(TEXT("World"), ESearchCase::IgnoreCase)
		|| ClassName.Equals(TEXT("ObjectRedirector"), ESearchCase::IgnoreCase);
}

bool SAssetReferenceInspectorWidget::TryGetPackageFilename(FName PackageName, FString& OutPackageFilename) const
{
	if (PackageName.IsNone())
	{
		return false;
	}

	const FString PackageNameString = PackageName.ToString();

	if (FPackageName::TryConvertLongPackageNameToFilename(PackageNameString, OutPackageFilename, FPackageName::GetAssetPackageExtension())
		&& GetExistingFileSizeBytes(OutPackageFilename) >= 0)
	{
		return true;
	}

	return FPackageName::TryConvertLongPackageNameToFilename(PackageNameString, OutPackageFilename, FPackageName::GetMapPackageExtension())
		&& GetExistingFileSizeBytes(OutPackageFilename) >= 0;
}

int64 SAssetReferenceInspectorWidget::GetPackageDiskSizeBytes(FName PackageName) const
{
	FString PackageFilename;
	if (!TryGetPackageFilename(PackageName, PackageFilename))
	{
		return 0;
	}

	int64 TotalSizeBytes = 0;

	const int64 PackageFileSize = GetExistingFileSizeBytes(PackageFilename);
	if (PackageFileSize > 0)
	{
		TotalSizeBytes += PackageFileSize;
	}

	const FString ExportFilename = FPaths::ChangeExtension(PackageFilename, TEXT("uexp"));
	const int64 ExportFileSize = GetExistingFileSizeBytes(ExportFilename);
	if (ExportFileSize > 0)
	{
		TotalSizeBytes += ExportFileSize;
	}

	const FString BulkDataFilename = FPaths::ChangeExtension(PackageFilename, TEXT("ubulk"));
	const int64 BulkDataFileSize = GetExistingFileSizeBytes(BulkDataFilename);
	if (BulkDataFileSize > 0)
	{
		TotalSizeBytes += BulkDataFileSize;
	}

	return TotalSizeBytes;
}

int64 SAssetReferenceInspectorWidget::GetExistingFileSizeBytes(const FString& Filename) const
{
	return Filename.IsEmpty()
		? -1
		: IFileManager::Get().FileSize(*Filename);
}

FString SAssetReferenceInspectorWidget::FormatSizeBytes(int64 SizeBytes) const
{
	if (SizeBytes >= 1024 * 1024)
	{
		return FString::Printf(TEXT("%.2f MB"), static_cast<double>(SizeBytes) / static_cast<double>(1024 * 1024));
	}

	if (SizeBytes >= 1024)
	{
		return FString::Printf(TEXT("%.2f KB"), static_cast<double>(SizeBytes) / static_cast<double>(1024));
	}

	return FString::Printf(TEXT("%lld B"), SizeBytes);
}

FString SAssetReferenceInspectorWidget::GetTreeNodeDisplayText(TSharedPtr<FAssetReferenceTreeNode> Item) const
{
	if (!Item.IsValid())
	{
		return FString(TEXT("Invalid Node"));
	}

	FString DisplayText = Item->DisplayName;

	if (!Item->ClassName.IsEmpty())
	{
		DisplayText = FString::Printf(TEXT("%s [%s]"), *DisplayText, *Item->ClassName);
	}

	if (Item->SizeBytes > 0)
	{
		DisplayText = FString::Printf(TEXT("%s (%s)"), *DisplayText, *FormatSizeBytes(Item->SizeBytes));
	}

	if (Item->bIsCircular)
	{
		DisplayText = FString::Printf(TEXT("%s [Circular]"), *DisplayText);
	}

	if (Item->bIsUnusedCandidate)
	{
		DisplayText = FString::Printf(TEXT("%s [Unused Candidate]"), *DisplayText);
	}

	return DisplayText;
}

TSharedRef<ITableRow> SAssetReferenceInspectorWidget::OnGenerateTreeRow(TSharedPtr<FAssetReferenceTreeNode> Item, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(STableRow<TSharedPtr<FAssetReferenceTreeNode>>, OwnerTable)
		[
			SNew(STextBlock)
				.Text(FText::FromString(GetTreeNodeDisplayText(Item)))
		];
}

void SAssetReferenceInspectorWidget::OnGetTreeChildren(TSharedPtr<FAssetReferenceTreeNode> Item, TArray<TSharedPtr<FAssetReferenceTreeNode>>& OutChildren) const
{
	if (Item.IsValid())
	{
		OutChildren.Append(Item->Children);
	}
}

bool SAssetReferenceInspectorWidget::TrySyncContentBrowserToPackage(FName PackageName) const
{
	if (PackageName.IsNone())
	{
		return false;
	}

	FAssetData AssetData;
	if (!TryGetPrimaryAssetDataForPackage(PackageName, AssetData))
	{
		return false;
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FAssetData> AssetsToSync;
	AssetsToSync.Add(AssetData);
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync);

	return true;
}
