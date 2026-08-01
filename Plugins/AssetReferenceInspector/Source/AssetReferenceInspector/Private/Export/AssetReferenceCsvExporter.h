#pragma once

#include "CoreMinimal.h"
#include "Analysis/AssetReferenceTypes.h"

class FAssetReferenceCsvExporter
{
public:
	static bool ExportToTimestampedCsv(const TArray<TSharedPtr<FAssetReferenceTreeNode>>& RootItems, EAssetReferenceMode Mode, FString& OutFilename, FString& OutErrorMessage);

private:
	static void AppendNodeRows(const TArray<TSharedPtr<FAssetReferenceTreeNode>>& Items, EAssetReferenceMode Mode, TArray<FString>& OutRows);

	static FString BuildCsvRow(const FAssetReferenceTreeNode& Node, EAssetReferenceMode Mode);
	static FString BuildUniqueExportFilename(const FString& ExportDirectory);
	static FString EscapeCsvField(const FString& Value);
	static FString GetModeString(const FAssetReferenceTreeNode& Node, EAssetReferenceMode Mode);
	static FString GetPackagePath(FName PackageName);
};
