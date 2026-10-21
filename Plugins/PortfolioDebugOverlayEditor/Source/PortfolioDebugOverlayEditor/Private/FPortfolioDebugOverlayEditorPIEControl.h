#pragma once

namespace PortfolioDebugOverlayEditorPIEControl
{
	void Initialize();
	void Shutdown();
	bool GetStartPaused();
	void SetStartPaused(bool bEnabled);
	bool HasSession();
	bool IsPaused();
	void Pause();
	void Resume();
}
