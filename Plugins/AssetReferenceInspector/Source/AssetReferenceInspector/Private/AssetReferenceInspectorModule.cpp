#include "AssetReferenceInspectorModule.h"

#include "AssetReferenceInspectorCommands.h"
#include "AssetReferenceInspectorStyle.h"
#include "UI/SAssetReferenceInspectorWidget.h"

#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"

#include "ToolMenus.h"

#include "Widgets/Docking/SDockTab.h"

static const FName AssetReferenceInspectorTabName(TEXT("AssetReferenceInspector"));

void FAssetReferenceInspectorModule::StartupModule()
{
	FAssetReferenceInspectorStyle::Initialize();
	FAssetReferenceInspectorCommands::Register();

	PluginCommands = MakeShared<FUICommandList>();

	PluginCommands->MapAction(
		FAssetReferenceInspectorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FAssetReferenceInspectorModule::OpenPluginWindow),
		FCanExecuteAction());

	// Create Delegate Object and Regist to Normad Tab Spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AssetReferenceInspectorTabName, FOnSpawnTab::CreateRaw(this, &FAssetReferenceInspectorModule::OnSpawnPluginTab))
		.SetDisplayName(FText::FromString(TEXT("Asset Reference Inspector")))
		.SetIcon(FSlateIcon(FAssetReferenceInspectorStyle::GetStyleSetName(), TEXT("AssetReferenceInspector.OpenPluginWindow")))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetReferenceInspectorModule::RegisterMenus));
}

void FAssetReferenceInspectorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AssetReferenceInspectorTabName);

	FAssetReferenceInspectorCommands::Unregister();
	FAssetReferenceInspectorStyle::Shutdown();
}

TSharedRef<SDockTab> FAssetReferenceInspectorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SAssetReferenceInspectorWidget)
		];
}

void FAssetReferenceInspectorModule::OpenPluginWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(AssetReferenceInspectorTabName);
}

void FAssetReferenceInspectorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window.PortfolioToolsDebugOverlay"));
	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("AssetReferenceInspector"));
	Section.AddMenuEntryWithCommandList(FAssetReferenceInspectorCommands::Get().OpenPluginWindow, PluginCommands);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User"));
	FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection(TEXT("AssetReferenceInspector"));
	FToolMenuEntry ToolbarEntry = FToolMenuEntry::InitToolBarButton(
		TEXT("OpenAssetReferenceInspectorToolbar"),
		FUIAction(FExecuteAction::CreateRaw(this, &FAssetReferenceInspectorModule::OpenPluginWindow)),
		FText::FromString(TEXT("Asset Inspector")),
		FText::FromString(TEXT("Open Asset Reference Inspector.")),
		FSlateIcon(FAssetReferenceInspectorStyle::GetStyleSetName(), TEXT("AssetReferenceInspector.OpenPluginWindow")));
	ToolbarEntry.StyleNameOverride = TEXT("AssetEditorToolbar");
	ToolbarSection.AddEntry(ToolbarEntry);
}

IMPLEMENT_MODULE(FAssetReferenceInspectorModule, AssetReferenceInspector)
