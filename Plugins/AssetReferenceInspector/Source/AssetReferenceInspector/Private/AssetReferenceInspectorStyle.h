#pragma once

#include "CoreMinimal.h"

class ISlateStyle;

class FAssetReferenceInspectorStyle
{
public:
	static void Initialize();
	static void Shutdown();

	static FName GetStyleSetName();
	static const ISlateStyle& Get();

private:
	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};
