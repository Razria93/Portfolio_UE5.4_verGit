#include "FPortfolioDebugOverlayEditorPIEControl.h"

#include "Editor.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet2/DebuggerCommands.h"

namespace
{
	TAutoConsoleVariable<int32> CVarStartPaused(
		TEXT("Portfolio.DebugOverlay.PIE.StartPaused"), 0,
		TEXT("Pause in-process PIE/SIE after initialization, before normal gameplay ticks. Set before Play. Editor-session only; does not block BeginPlay."));
	FDelegateHandle PostPIEStartedHandle;

	void OnPostPIEStarted(bool bSimulating)
	{
		if (PortfolioDebugOverlayEditorPIEControl::GetStartPaused())
		{
			PortfolioDebugOverlayEditorPIEControl::Pause();
		}
	}

	FAutoConsoleCommand PauseCommand(TEXT("Portfolio.DebugOverlay.PIE.Pause"),
		TEXT("Pause the current in-process PIE/SIE session."),
		FConsoleCommandDelegate::CreateStatic(&PortfolioDebugOverlayEditorPIEControl::Pause));
	FAutoConsoleCommand ResumeCommand(TEXT("Portfolio.DebugOverlay.PIE.Resume"),
		TEXT("Resume the current in-process PIE/SIE session."),
		FConsoleCommandDelegate::CreateStatic(&PortfolioDebugOverlayEditorPIEControl::Resume));
}

void PortfolioDebugOverlayEditorPIEControl::Initialize()
{
	if (!PostPIEStartedHandle.IsValid())
	{
		PostPIEStartedHandle = FEditorDelegates::PostPIEStarted.AddStatic(&OnPostPIEStarted);
	}
}

void PortfolioDebugOverlayEditorPIEControl::Shutdown()
{
	FEditorDelegates::PostPIEStarted.Remove(PostPIEStartedHandle);
	PostPIEStartedHandle.Reset();
}

bool PortfolioDebugOverlayEditorPIEControl::GetStartPaused()
{
	return CVarStartPaused.GetValueOnGameThread() != 0;
}

void PortfolioDebugOverlayEditorPIEControl::SetStartPaused(bool bEnabled)
{
	CVarStartPaused->Set(bEnabled ? 1 : 0, ECVF_SetByConsole);
}

bool PortfolioDebugOverlayEditorPIEControl::HasSession()
{
	return GEditor && IsValid(GEditor->PlayWorld);
}

bool PortfolioDebugOverlayEditorPIEControl::IsPaused()
{
	return HasSession() && GEditor->PlayWorld->bDebugPauseExecution;
}

void PortfolioDebugOverlayEditorPIEControl::Pause()
{
	if (HasSession() && !IsPaused())
	{
		FPlayWorldCommandCallbacks::PausePlaySession_Clicked();
	}
}

void PortfolioDebugOverlayEditorPIEControl::Resume()
{
	if (IsPaused())
	{
		FPlayWorldCommandCallbacks::ResumePlaySession_Clicked();
	}
}
