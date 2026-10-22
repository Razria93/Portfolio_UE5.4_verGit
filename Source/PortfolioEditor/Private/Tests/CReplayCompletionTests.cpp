#include "Misc/AutomationTest.h"
#include "Capture/CReplayCompletion.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReplayCompletionTest, "Portfolio.Capture.Replay.CompletionContract", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FReplayCompletionTest::RunTest(const FString&)
{
    const FString Done = TEXT("stop_reason=requested_stop\nframes=complete\naudio=valid_pcm_nonzero\nmanifest=written\n");
    const FString Summary = TEXT("stop_reason=requested_stop\nrequested=4\nreturned=4\nsaved=4\npending_at_finalize=0\nframes=complete\naudio=valid_pcm_nonzero\n");
    auto Valid = [](const FString& D, const FString& S) { return PortfolioReplay::ValidateCompletion(D, S).IsEmpty(); };
    TestTrue(TEXT("Complete requested stop with consistent accounting succeeds"), Valid(Done, Summary));
    TestTrue(TEXT("Windows newlines are accepted"), Valid(Done.Replace(TEXT("\n"), TEXT("\r\n")), Summary.Replace(TEXT("\n"), TEXT("\r\n"))));
    TestFalse(TEXT("Busy zero without completion files is not success"), Valid(TEXT(""), TEXT("")));
    for (const TCHAR* Reason : {TEXT("viewport_geometry_changed"), TEXT("end_pie"), TEXT("world_unavailable")})
        TestFalse(Reason, Valid(Done.Replace(TEXT("requested_stop"), Reason), Summary.Replace(TEXT("requested_stop"), Reason)));
    for (const TCHAR* State : {TEXT("incomplete_writer_timeout"), TEXT("complete_with_drops_or_errors")})
        TestFalse(State, Valid(Done.Replace(TEXT("frames=complete"), *(FString(TEXT("frames=")) + State)), Summary.Replace(TEXT("frames=complete"), *(FString(TEXT("frames=")) + State))));
    for (const TCHAR* State : {TEXT("silent_pcm"), TEXT("duration_mismatch"), TEXT("invalid_wav"), TEXT("unverified_export_timeout")})
        TestFalse(State, Valid(Done.Replace(TEXT("valid_pcm_nonzero"), State), Summary.Replace(TEXT("valid_pcm_nonzero"), State)));
    TestTrue(TEXT("Explicit silent-take policy accepts silent PCM, not fabricated audio evidence"),
        PortfolioReplay::ValidateCompletion(Done.Replace(TEXT("valid_pcm_nonzero"), TEXT("silent_pcm")), Summary.Replace(TEXT("valid_pcm_nonzero"), TEXT("silent_pcm")), true).IsEmpty());
    TestFalse(TEXT("Silent-take policy cannot accept an audio timeout"),
        PortfolioReplay::ValidateCompletion(Done.Replace(TEXT("valid_pcm_nonzero"), TEXT("unverified_export_timeout")), Summary.Replace(TEXT("valid_pcm_nonzero"), TEXT("unverified_export_timeout")), true).IsEmpty());
    TestFalse(TEXT("Manifest write failure rejects success"), Valid(Done.Replace(TEXT("manifest=written"), TEXT("manifest=write_failed")), Summary));
    TestFalse(TEXT("Conflicting completion files reject success"), Valid(Done, Summary.Replace(TEXT("requested_stop"), TEXT("end_pie"))));
    TestFalse(TEXT("Pending writer rejects success"), Valid(Done, Summary.Replace(TEXT("pending_at_finalize=0"), TEXT("pending_at_finalize=1"))));
    TestFalse(TEXT("Missing frames reject success"), Valid(Done, Summary.Replace(TEXT("saved=4"), TEXT("saved=3"))));
    TestFalse(TEXT("Malformed count rejects success"), Valid(Done, Summary.Replace(TEXT("saved=4"), TEXT("saved=4x"))));
    TestFalse(TEXT("Empty take rejects success"), Valid(Done, Summary.Replace(TEXT("=4"), TEXT("=0"))));
    TestFalse(TEXT("Duplicate field rejects success"), Valid(Done + TEXT("frames=complete\n"), Summary));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReplayScenarioGoalTest, "Portfolio.Capture.Replay.ScenarioGoal", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FReplayScenarioGoalTest::RunTest(const FString&)
{
    using PortfolioReplay::ValidateScenarioGoal;
    TestFalse(TEXT("No observed equipment transition is not an equip take"), ValidateScenarioGoal(TEXT("equip"), false, true, -1).IsEmpty());
    TestFalse(TEXT("Equipping alone is not an equip and unequip take"), ValidateScenarioGoal(TEXT("equip"), true, false, -1).IsEmpty());
    TestTrue(TEXT("Both equipment transitions complete the goal"), ValidateScenarioGoal(TEXT("equip"), true, true, -1).IsEmpty());
    for (const TCHAR* Mode : {TEXT("combo"), TEXT("combo3")})
    {
        for (int32 Index : {-1, 0, 1}) TestFalse(TEXT("Absent or short combo fails the three-hit goal"), ValidateScenarioGoal(Mode, false, false, Index).IsEmpty());
        TestTrue(TEXT("Third combo action observed"), ValidateScenarioGoal(Mode, false, false, 2).IsEmpty());
    }
    TestFalse(TEXT("First hit alone is not combo2"), ValidateScenarioGoal(TEXT("combo2"), false, false, 0).IsEmpty());
    TestTrue(TEXT("Second combo action observed"), ValidateScenarioGoal(TEXT("combo2"), false, false, 1).IsEmpty());
    TestTrue(TEXT("Execution uses its own session path, not a combo goal"), ValidateScenarioGoal(TEXT("execution"), false, false, -1).IsEmpty());
    return true;
}
#endif
