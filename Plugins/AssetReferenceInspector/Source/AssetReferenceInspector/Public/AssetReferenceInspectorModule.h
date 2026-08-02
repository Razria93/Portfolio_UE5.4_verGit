#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FAssetReferenceInspectorModule : public IModuleInterface
{
public:
	// IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// Tab
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	void OpenPluginWindow();

	// Menu
	void RegisterMenus();

private:
	// Commands
	TSharedPtr<class FUICommandList> PluginCommands;
};
