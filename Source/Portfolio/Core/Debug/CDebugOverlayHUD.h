#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CDebugOverlayHUD.generated.h"

UCLASS()
class PORTFOLIO_API ACDebugOverlayHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
