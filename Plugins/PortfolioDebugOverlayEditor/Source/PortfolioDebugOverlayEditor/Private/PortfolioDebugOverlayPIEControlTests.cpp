#if WITH_DEV_AUTOMATION_TESTS
#include "FPortfolioDebugOverlayEditorPIEControl.h"
#include "Editor.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace PIEControl = PortfolioDebugOverlayEditorPIEControl;

class FVerifyCapturePIESession : public IAutomationLatentCommand
{
public:
	FVerifyCapturePIESession(FAutomationTestBase* InTest, bool bInStartPaused, bool bInRestore)
		: Test(InTest), bStartPaused(bInStartPaused), bRestore(bInRestore) {}

	virtual bool Update() override
	{
		if (Phase == 0)
		{
			PIEControl::SetStartPaused(bStartPaused);
			FStartPIECommand(false).Update();
			Deadline = FPlatformTime::Seconds() + 30.0;
			Phase = 1;
			return false;
		}
		if (FPlatformTime::Seconds() > Deadline)
		{
			Test->AddError(TEXT("Timed out waiting for capture preparation PIE lifecycle"));
			GEditor->RequestEndPlayMap();
			PIEControl::SetStartPaused(bRestore);
			return true;
		}
		if (Phase == 1)
		{
			if (!PIEControl::HasSession()) return false;
			Test->TestEqual(TEXT("Initial PIE pause follows StartPaused"), PIEControl::IsPaused(), bStartPaused);
			PIEControl::Pause();
			Test->TestTrue(TEXT("Manual Pause works"), PIEControl::IsPaused());
			WorldTime = GEditor->PlayWorld->GetTimeSeconds();
			WaitUntil = FPlatformTime::Seconds() + 0.3;
			Phase = 2;
			return false;
		}
		if (Phase == 2)
		{
			if (FPlatformTime::Seconds() < WaitUntil) return false;
			Test->TestEqual(TEXT("Gameplay time stays frozen during preparation"), GEditor->PlayWorld->GetTimeSeconds(), WorldTime);
			PIEControl::Resume();
			Test->TestFalse(TEXT("Resume clears editor pause"), PIEControl::IsPaused());
			WaitUntil = FPlatformTime::Seconds() + 0.3;
			Phase = 3;
			return false;
		}
		if (Phase == 3)
		{
			if (FPlatformTime::Seconds() < WaitUntil) return false;
			Test->TestTrue(TEXT("Gameplay time advances after Resume"), GEditor->PlayWorld->GetTimeSeconds() > WorldTime);
			GEditor->RequestEndPlayMap();
			Phase = 4;
			return false;
		}
		if (PIEControl::HasSession()) return false;
		PIEControl::SetStartPaused(bRestore);
		return true;
	}
private:
	FAutomationTestBase* Test;
	bool bStartPaused;
	bool bRestore;
	int32 Phase = 0;
	double Deadline = 0;
	double WaitUntil = 0;
	double WorldTime = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPortfolioCapturePIEControlTest,
	"Portfolio.DebugOverlay.CapturePreparation.PIELifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPortfolioCapturePIEControlTest::RunTest(const FString& Parameters)
{
	if (PIEControl::HasSession())
	{
		AddError(TEXT("Run this test outside an existing PIE session"));
		return false;
	}
	UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
	World->GetWorldSettings()->DefaultGameMode = AGameModeBase::StaticClass();
	const bool bOriginalSetting = PIEControl::GetStartPaused();
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyCapturePIESession(this, true, bOriginalSetting));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyCapturePIESession(this, false, bOriginalSetting));
	return true;
}
#endif
