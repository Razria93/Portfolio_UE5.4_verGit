#include "Export/AssetReferenceCsvExporter.h"

#include "HAL/FileManager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"

bool FAssetReferenceCsvExporter::ExportToTimestampedCsv(const TArray<TSharedPtr<FAssetReferenceTreeNode>>& RootItems, EAssetReferenceMode Mode, FString& OutFilename, FString& OutErrorMessage)
{
	TArray<FString> Rows;
	Rows.Add(TEXT("AssetName,PackageName,Class,Path,Depth,SizeBytes,Mode,ParentPackage,IsCircular,IsUnusedCandidate"));

	AppendNodeRows(RootItems, Mode, Rows);

	if (Rows.Num() <= 1)
	{
		OutErrorMessage = TEXT("No exportable Asset rows found.");
		return false;
	}

	const FString ExportDirectory = FPaths::ProjectSavedDir() / TEXT("AssetReferenceInspector");
	if (!IFileManager::Get().MakeDirectory(*ExportDirectory, true))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to create export directory: %s"), *ExportDirectory);
		return false;
	}

	OutFilename = BuildUniqueExportFilename(ExportDirectory);

	const FString CsvContent = FString::Join(Rows, TEXT("\n"));
	if (!FFileHelper::SaveStringToFile(CsvContent, *OutFilename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to save CSV file: %s"), *OutFilename);
		return false;
	}

	return true;
}

void FAssetReferenceCsvExporter::AppendNodeRows(const TArray<TSharedPtr<FAssetReferenceTreeNode>>& Items, EAssetReferenceMode Mode, TArray<FString>& OutRows)
{
	for (const TSharedPtr<FAssetReferenceTreeNode>& Item : Items)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		if (!Item->PackageName.IsNone())
		{
			OutRows.Add(BuildCsvRow(*Item, Mode));
		}

		AppendNodeRows(Item->Children, Mode, OutRows);
	}
}

FString FAssetReferenceCsvExporter::BuildCsvRow(const FAssetReferenceTreeNode& Node, EAssetReferenceMode Mode)
{
	TArray<FString> Fields;
	Fields.Reserve(10);

	Fields.Add(EscapeCsvField(Node.DisplayName));
	Fields.Add(EscapeCsvField(Node.PackageName.ToString()));
	Fields.Add(EscapeCsvField(Node.ClassName));
	Fields.Add(EscapeCsvField(GetPackagePath(Node.PackageName)));
	Fields.Add(FString::FromInt(Node.Depth));
	Fields.Add(FString::Printf(TEXT("%lld"), Node.SizeBytes));
	Fields.Add(EscapeCsvField(GetModeString(Node, Mode)));
	Fields.Add(EscapeCsvField(Node.ParentPackageName.IsNone() ? FString() : Node.ParentPackageName.ToString()));
	Fields.Add(Node.bIsCircular ? TEXT("true") : TEXT("false"));
	Fields.Add(Node.bIsUnusedCandidate ? TEXT("true") : TEXT("false"));

	return FString::Join(Fields, TEXT(","));
}

FString FAssetReferenceCsvExporter::BuildUniqueExportFilename(const FString& ExportDirectory)
{
	const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString BaseFilename = ExportDirectory / FString::Printf(TEXT("AssetReferenceReport_%s"), *Timestamp);
	const FString DefaultFilename = BaseFilename + TEXT(".csv");

	if (!IFileManager::Get().FileExists(*DefaultFilename))
	{
		return DefaultFilename;
	}

	for (int32 SuffixIndex = 1; SuffixIndex <= 999; ++SuffixIndex)
	{
		const FString CandidateFilename = FString::Printf(TEXT("%s_%03d.csv"), *BaseFilename, SuffixIndex);
		if (!IFileManager::Get().FileExists(*CandidateFilename))
		{
			return CandidateFilename;
		}
	}

	return ExportDirectory / FString::Printf(TEXT("AssetReferenceReport_%s_%lld.csv"), *Timestamp, FDateTime::Now().GetTicks());
}

FString FAssetReferenceCsvExporter::EscapeCsvField(const FString& Value)
{
	FString EscapedValue = Value;
	EscapedValue.ReplaceInline(TEXT("\""), TEXT("\"\""), ESearchCase::CaseSensitive);

	if (EscapedValue.Contains(TEXT(",")) || EscapedValue.Contains(TEXT("\"")) || EscapedValue.Contains(TEXT("\n")) || EscapedValue.Contains(TEXT("\r")))
	{
		return FString::Printf(TEXT("\"%s\""), *EscapedValue);
	}

	return EscapedValue;
}

FString FAssetReferenceCsvExporter::GetModeString(const FAssetReferenceTreeNode& Node, EAssetReferenceMode Mode)
{
	if (Node.bIsUnusedCandidate)
	{
		return FString(TEXT("UnusedCandidates"));
	}

	return Mode == EAssetReferenceMode::Referencers
		? FString(TEXT("Referencers"))
		: FString(TEXT("Dependencies"));
}

FString FAssetReferenceCsvExporter::GetPackagePath(FName PackageName)
{
	if (PackageName.IsNone())
	{
		return FString();
	}

	return FPackageName::GetLongPackagePath(PackageName.ToString());
}
