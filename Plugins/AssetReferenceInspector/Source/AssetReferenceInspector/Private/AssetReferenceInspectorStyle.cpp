#include "AssetReferenceInspectorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FAssetReferenceInspectorStyle::StyleInstance = nullptr;

void FAssetReferenceInspectorStyle::Initialize()
{
	if (StyleInstance.IsValid())
	{
		return;
	}

	StyleInstance = MakeShared<FSlateStyleSet>(GetStyleSetName());

	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("AssetReferenceInspector"));
	if (Plugin.IsValid())
	{
		StyleInstance->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}

	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	StyleInstance->Set(
		TEXT("AssetReferenceInspector.OpenPluginWindow"),
		new FSlateImageBrush(StyleInstance->RootToContentDir(TEXT("Icon128"), TEXT(".png")), Icon40x40));
	StyleInstance->Set(
		TEXT("AssetReferenceInspector.OpenPluginWindow.Small"),
		new FSlateImageBrush(StyleInstance->RootToContentDir(TEXT("Icon128"), TEXT(".png")), Icon16x16));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
}

void FAssetReferenceInspectorStyle::Shutdown()
{
	if (!StyleInstance.IsValid())
	{
		return;
	}

	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAssetReferenceInspectorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AssetReferenceInspectorStyle"));
	return StyleSetName;
}

const ISlateStyle& FAssetReferenceInspectorStyle::Get()
{
	return *StyleInstance;
}
