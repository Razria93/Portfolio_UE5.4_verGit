#include "PortfolioDebugOverlayEditorModule.h"

#include "SPortfolioDebugOverlayEditorWidget.h"

#include "Framework/Commands/UIAction.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FPortfolioDebugOverlayEditorModule"

namespace
{
	// ===== Constants =====

	static const FName DebugOverlayTabName(TEXT("PortfolioDebugOverlayEditor"));
}

// ===== Module Lifecycle =====

void FPortfolioDebugOverlayEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		DebugOverlayTabName,
		FOnSpawnTab::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::SpawnDebugOverlayTab))
		.SetDisplayName(LOCTEXT("DebugOverlayTabDisplayName", "Debug Overlay"))
		.SetTooltipText(LOCTEXT("DebugOverlayTabTooltip", "Open Portfolio Debug Overlay settings."))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::RegisterMenus));
}

void FPortfolioDebugOverlayEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DebugOverlayTabName);
}

// ===== Menu Registration =====

void FPortfolioDebugOverlayEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped ownerScoped(this);

	UToolMenu* menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
	FToolMenuSection& section = menu->FindOrAddSection(TEXT("PortfolioTools"));
	section.AddSubMenu(
		TEXT("PortfolioToolsDebugOverlay"),
		LOCTEXT("PortfolioToolsSubMenuLabel", "Portfolio Tools"),
		LOCTEXT("PortfolioToolsSubMenuTooltip", "Open Portfolio editor tools."),
		FNewToolMenuDelegate::CreateLambda([this](UToolMenu* subMenu)
		{
			FToolMenuSection& subMenuSection = subMenu->FindOrAddSection(TEXT("DebugOverlay"));
			subMenuSection.AddMenuEntry(
				TEXT("OpenPortfolioDebugOverlayPanel"),
				LOCTEXT("OpenDebugOverlayPanelLabel", "Debug Overlay"),
				LOCTEXT("OpenDebugOverlayPanelTooltip", "Open Portfolio Debug Overlay settings panel."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::OpenDebugOverlayPanel)));
		}));

	UToolMenu* toolbarMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User"));
	FToolMenuSection& toolbarSection = toolbarMenu->FindOrAddSection(TEXT("PortfolioDebugOverlay"));
	FToolMenuEntry toolbarEntry = FToolMenuEntry::InitToolBarButton(
		TEXT("OpenPortfolioDebugOverlayPanelToolbar"),
		FUIAction(FExecuteAction::CreateRaw(this, &FPortfolioDebugOverlayEditorModule::OpenDebugOverlayPanel)),
		LOCTEXT("OpenDebugOverlayPanelToolbarLabel", "Debug Overlay"),
		LOCTEXT("OpenDebugOverlayPanelToolbarTooltip", "Open Debug Overlay panel"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Settings")));
	toolbarEntry.StyleNameOverride = TEXT("AssetEditorToolbar");
	toolbarSection.AddEntry(toolbarEntry);
}

// ===== Tab Spawning =====

void FPortfolioDebugOverlayEditorModule::OpenDebugOverlayPanel()
{
	FGlobalTabmanager::Get()->TryInvokeTab(DebugOverlayTabName);
}

TSharedRef<SDockTab> FPortfolioDebugOverlayEditorModule::SpawnDebugOverlayTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SPortfolioDebugOverlayEditorWidget)
		];
}

IMPLEMENT_MODULE(FPortfolioDebugOverlayEditorModule, PortfolioDebugOverlayEditor)

#undef LOCTEXT_NAMESPACE
