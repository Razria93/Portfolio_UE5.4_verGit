#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CDebugOverlayHUD.generated.h"

class ACEnemy;
struct FDebugOverlayFocusViewData;

UCLASS()
class PORTFOLIO_API ACDebugOverlayHUD : public AHUD
{
	GENERATED_BODY()

public:
	// HUD Rendering
	virtual void DrawHUD() override;
};
