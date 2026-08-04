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

#if !UE_BUILD_SHIPPING
private:
	// Enemy Resolution
	ACEnemy* ResolveDisplayEnemy(FDebugOverlayFocusViewData& OutFocusViewData);
	ACEnemy* ResolveFocusComponentEnemy(FDebugOverlayFocusViewData& OutFocusViewData) const;
#endif

public:
	// Rendering
	virtual void DrawHUD() override;
};
