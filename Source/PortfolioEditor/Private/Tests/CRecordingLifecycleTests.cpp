#include "Misc/AutomationTest.h"
#include "Capture/CRecordingLifecycle.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecordingSettingsRestoreTest, "Portfolio.Capture.Recording.SettingsRestore", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRecordingSettingsRestoreTest::RunTest(const FString&)
{
    auto& Manager = IConsoleManager::Get();
    const TCHAR* Name = TEXT("Portfolio.Tests.RecordingTemporarySetting");
    IConsoleVariable* Variable = Manager.RegisterConsoleVariable(Name, 7, TEXT("Recording regression fixture"));
    for (EConsoleVariableFlags Priority : {ECVF_SetByConstructor, ECVF_SetByCode, ECVF_SetByConsole})
    {
        Variable->Set(7, Priority);
        PortfolioRecording::FCVarOverride Override;
        Override.Apply(Name, 0);
        TestEqual(TEXT("Capture overrides even a console-priority setting"), Variable->GetInt(), 0);
        TestEqual(TEXT("Capture never promotes priority"), uint32(Variable->GetFlags() & ECVF_SetByMask), uint32(Priority));
        Override.Restore();
        TestEqual(TEXT("Stop restores value"), Variable->GetInt(), 7);
        TestEqual(TEXT("Stop restores priority"), uint32(Variable->GetFlags() & ECVF_SetByMask), uint32(Priority));
        Override.Restore();
        TestEqual(TEXT("Repeated stop is harmless"), Variable->GetInt(), 7);
    }
    PortfolioRecording::FCVarOverride Override;
    Override.Apply(Name, 0);
    Variable->Set(9, ECVF_SetByConsole);
    Override.Restore();
    TestEqual(TEXT("New user choice during recording survives stop"), Variable->GetInt(), 9);
    Manager.UnregisterConsoleObject(Name, false);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecordingExportBoundaryTest, "Portfolio.Capture.Recording.ExportBoundaries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRecordingExportBoundaryTest::RunTest(const FString&)
{
    PortfolioRecording::FExportProgress Progress;
    Progress.StartedAt = 100;
    TestFalse(TEXT("Missing audio and pending JPEG do not finish"), Progress.Poll(100, 1, -1));
    for (int32 I = 0; I < 4; ++I) Progress.Poll(101 + I * .25, 1, 1000);
    TestTrue(TEXT("WAV stable after three unchanged observations"), Progress.HasStableAudio());
    TestFalse(TEXT("Stable WAV cannot hide pending JPEG"), Progress.Poll(102, 1, 1000));
    TestTrue(TEXT("Completed JPEG plus stable WAV finishes"), Progress.Poll(102.25, 0, 1000));
    TestFalse(TEXT("Growing WAV resets stability"), Progress.Poll(103, 0, 2000));
    TestFalse(TEXT("Before deadline a stalled writer remains pending"), Progress.Poll(114.999, 1, -1));
    TestTrue(TEXT("Exact deadline ends stalled JPEG and missing WAV wait"), Progress.Poll(115, 1, -1));
    TestTrue(TEXT("Timeout is explicit, not successful completion"), Progress.HasTimedOut(115));
    TestFalse(TEXT("Missing WAV never becomes stable"), Progress.HasStableAudio());
    PortfolioRecording::FExportProgress Empty;
    Empty.StartedAt = 20;
    TestFalse(TEXT("Zero-byte WAV is not success even without pending JPEG"), Empty.Poll(21, 0, 0));
    TestTrue(TEXT("WAV-only timeout is bounded"), Empty.Poll(35, 0, 0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRecordingGeometryTest, "Portfolio.Capture.Recording.FixedGeometry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRecordingGeometryTest::RunTest(const FString&)
{
    using PortfolioRecording::FRecordingGeometry;
    const FRecordingGeometry Initial{FIntPoint(1280, 720), FIntPoint(3, 35), FIntPoint(1286, 758), 123};
    TestTrue(TEXT("Unchanged window-local viewport remains valid"), Initial.Matches(Initial));
    auto Current = Initial;
    Current.ViewportSize = FIntPoint(1600, 900);
    TestFalse(TEXT("Enlargement cannot silently crop the old size"), Initial.Matches(Current));
    Current = Initial; Current.ViewportSize = FIntPoint(640, 360);
    TestFalse(TEXT("Shrink ends the take instead of repeated drops"), Initial.Matches(Current));
    Current = Initial; Current.CropOrigin.X += 100;
    TestFalse(TEXT("Same-size viewport layout shift changes the crop"), Initial.Matches(Current));
    Current = Initial; Current.WindowId += 1;
    TestFalse(TEXT("Reparenting to a same-size window changes ownership"), Initial.Matches(Current));
    Current = Initial; Current.BackBufferSize.X += 10;
    TestFalse(TEXT("Backbuffer-only resize is a geometry change"), Initial.Matches(Current));
    TestFalse(TEXT("Unavailable geometry never validates itself"), FRecordingGeometry{}.Matches(FRecordingGeometry{}));
    return true;
}
#endif
