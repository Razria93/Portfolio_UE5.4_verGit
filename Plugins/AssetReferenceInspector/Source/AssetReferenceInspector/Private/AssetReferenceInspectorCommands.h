#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

class FAssetReferenceInspectorCommands : public TCommands<FAssetReferenceInspectorCommands>
{
public:
	FAssetReferenceInspectorCommands();

	// TCommands
	virtual void RegisterCommands() override;

public:
	// Commands
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};
