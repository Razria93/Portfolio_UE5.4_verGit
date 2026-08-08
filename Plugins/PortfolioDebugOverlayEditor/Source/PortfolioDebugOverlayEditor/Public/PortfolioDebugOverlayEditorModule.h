#pragma once

#include "Modules/ModuleInterface.h"
#include "Templates/SharedPointer.h"

class FSpawnTabArgs;
class SDockTab;

class FPortfolioDebugOverlayEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// ===== Menu Registration =====

	void RegisterMenus();

	// ===== Tab Spawning =====

	void OpenDebugOverlayPanel();
	TSharedRef<SDockTab> SpawnDebugOverlayTab(const FSpawnTabArgs& SpawnTabArgs);
};
