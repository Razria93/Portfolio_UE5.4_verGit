#pragma once

#include "CoreMinimal.h"

class ACDebugOverlayHUD;
class UCanvas;
struct FDebugOverlayTextPanels;

class FDebugOverlayCanvasRenderer
{
public:
	static void Draw(ACDebugOverlayHUD& InHud, UCanvas* InCanvas, const FDebugOverlayTextPanels& InTextPanels);
};
