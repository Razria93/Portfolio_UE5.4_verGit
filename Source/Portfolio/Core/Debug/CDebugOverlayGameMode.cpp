#include "Core/Debug/CDebugOverlayGameMode.h"

#include "Core/Debug/CDebugOverlayHUD.h"

ACDebugOverlayGameMode::ACDebugOverlayGameMode()
{
#if !UE_BUILD_SHIPPING
	HUDClass = ACDebugOverlayHUD::StaticClass();
#endif
}
