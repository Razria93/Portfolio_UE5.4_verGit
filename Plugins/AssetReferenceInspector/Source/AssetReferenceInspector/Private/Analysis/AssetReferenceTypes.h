
#pragma once

#include "CoreMinimal.h"

enum class EAssetReferenceMode : uint8
{
	Dependencies,
	Referencers
};

struct FAssetReferenceAnalysisOptions
{
	EAssetReferenceMode Mode = EAssetReferenceMode::Dependencies;
	int32 MaxDepth = 2;
	FString PathFilter = TEXT("/Game/");
	FString ClassFilter;
	bool bIncludeEngineContent = false;
	bool bIncludePluginContent = false;
};

struct FAssetReferenceTreeNode
{
	explicit FAssetReferenceTreeNode(FString InDisplayName, FName InPackageName = NAME_None, int32 InDepth = 0, bool bInIsCircular = false, FString InClassName = FString(), int64 InSizeBytes = 0, bool bInIsUnusedCandidate = false, FName InParentPackageName = NAME_None)
		: DisplayName(MoveTemp(InDisplayName))
		, PackageName(InPackageName)
		, ParentPackageName(InParentPackageName)
		, Depth(InDepth)
		, bIsCircular(bInIsCircular)
		, ClassName(MoveTemp(InClassName))
		, SizeBytes(InSizeBytes)
		, bIsUnusedCandidate(bInIsUnusedCandidate)
	{
	}

	FString DisplayName;
	FName PackageName;
	FName ParentPackageName;
	int32 Depth = 0;
	bool bIsCircular = false;
	FString ClassName;
	int64 SizeBytes = 0;
	bool bIsUnusedCandidate = false;
	TArray<TSharedPtr<FAssetReferenceTreeNode>> Children;
};
