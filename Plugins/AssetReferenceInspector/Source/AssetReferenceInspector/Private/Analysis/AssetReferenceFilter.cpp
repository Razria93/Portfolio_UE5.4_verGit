#include "Analysis/AssetReferenceFilter.h"

#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"

bool FAssetReferenceFilter::ShouldPassRelationFilters(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName)
{
	return DoesContentSourcePassFilter(AnalysisOptions, PackageName)
		&& DoesPathPassFilter(AnalysisOptions, PackageName)
		&& DoesAssetClassPassFilter(AnalysisOptions, PackageName);
}

bool FAssetReferenceFilter::DoesContentSourcePassFilter(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName)
{
	if (IsEngineContentPackage(PackageName) && !AnalysisOptions.bIncludeEngineContent)
	{
		return false;
	}

	if (IsPluginContentPackage(PackageName) && !AnalysisOptions.bIncludePluginContent)
	{
		return false;
	}

	return true;
}

bool FAssetReferenceFilter::DoesPathPassFilter(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName)
{
	const FString PathFilter = AnalysisOptions.PathFilter.TrimStartAndEnd();

	if (PathFilter.IsEmpty() || PackageName.ToString().StartsWith(PathFilter))
	{
		return true;
	}

	if (AnalysisOptions.bIncludeEngineContent && IsEngineContentPackage(PackageName))
	{
		return true;
	}

	if (AnalysisOptions.bIncludePluginContent && IsPluginContentPackage(PackageName))
	{
		return true;
	}

	return false;
}

bool FAssetReferenceFilter::DoesAssetClassPassFilter(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName)
{
	const FString ClassFilter = AnalysisOptions.ClassFilter.TrimStartAndEnd();

	if (ClassFilter.IsEmpty())
	{
		return true;
	}

	FAssetData AssetData;
	if (!TryGetPrimaryAssetDataForPackage(PackageName, AssetData))
	{
		return false;
	}

	const FString ClassPath = AssetData.AssetClassPath.ToString();
	const FString ClassName = AssetData.AssetClassPath.GetAssetName().ToString();

	return ClassPath.Contains(ClassFilter, ESearchCase::IgnoreCase)
		|| ClassName.Contains(ClassFilter, ESearchCase::IgnoreCase);
}

bool FAssetReferenceFilter::IsEngineContentPackage(FName PackageName)
{
	return PackageName.ToString().StartsWith(TEXT("/Engine/"));
}

bool FAssetReferenceFilter::IsPluginContentPackage(FName PackageName)
{
	const FString PackageNameString = PackageName.ToString();

	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPluginsWithContent())
	{
		FString MountedAssetPath = Plugin->GetMountedAssetPath();
		MountedAssetPath.RemoveFromEnd(TEXT("/"));

		if (!MountedAssetPath.IsEmpty()
			&& PackageNameString.StartsWith(MountedAssetPath + TEXT("/")))
		{
			return true;
		}
	}

	return false;
}

bool FAssetReferenceFilter::TryGetPrimaryAssetDataForPackage(FName PackageName, FAssetData& OutAssetData)
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
