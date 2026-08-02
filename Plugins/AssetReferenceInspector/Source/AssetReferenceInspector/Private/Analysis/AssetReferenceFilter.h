#pragma once

#include "CoreMinimal.h"
#include "Analysis/AssetReferenceTypes.h"

struct FAssetData;

class FAssetReferenceFilter
{
public:
	// Entry point
	static bool ShouldPassRelationFilters(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName);

private:
	// Predicate helpers
	static bool DoesContentSourcePassFilter(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName);
	static bool DoesPathPassFilter(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName);
	static bool DoesAssetClassPassFilter(const FAssetReferenceAnalysisOptions& AnalysisOptions, FName PackageName);

	// Content source
	static bool IsEngineContentPackage(FName PackageName);
	static bool IsPluginContentPackage(FName PackageName);

	// Asset data
	static bool TryGetPrimaryAssetDataForPackage(FName PackageName, FAssetData& OutAssetData);
};
