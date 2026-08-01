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
	void RegisterMenus();
	void OpenDebugOverlayPanel();
	TSharedRef<SDockTab> SpawnDebugOverlayTab(const FSpawnTabArgs& SpawnTabArgs);
};
