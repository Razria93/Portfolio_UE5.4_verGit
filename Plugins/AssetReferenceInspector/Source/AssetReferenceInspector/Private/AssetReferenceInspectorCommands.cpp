#include "AssetReferenceInspectorCommands.h"

#define LOCTEXT_NAMESPACE "FAssetReferenceInspectorCommands"

FAssetReferenceInspectorCommands::FAssetReferenceInspectorCommands()
	: TCommands<FAssetReferenceInspectorCommands>(
		TEXT("AssetReferenceInspector"),
		FText::FromString(TEXT("Asset Reference Inspector")),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FAssetReferenceInspectorCommands::RegisterCommands()
{
	UI_COMMAND(
		OpenPluginWindow,
		"Asset Reference Inspector",
		"Open the Asset Reference Inspector window.",
		EUserInterfaceActionType::Button,
		FInputChord());
}

#undef LOCTEXT_NAMESPACE
