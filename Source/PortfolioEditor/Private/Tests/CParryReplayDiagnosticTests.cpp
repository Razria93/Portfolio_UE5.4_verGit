#include "Misc/AutomationTest.h"
#include "Capture/CParryReplayDiagnostic.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParryReplayDiagnosticTest, "Portfolio.Capture.Replay.ParryDiagnostic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FParryReplayDiagnosticTest::RunTest(const FString&)
{
    PortfolioReplay::FParryReplayDiagnostic D;
    auto Expect = [&](const TCHAR* Name) { TestEqual(Name, FString(D.Classify()), FString(Name)); };
    Expect(TEXT("IncompleteObservation"));
    D.Generation = 10; D.Index = 0;
    TestTrue(TEXT("Same first attack matches"), D.Matches(10, 0));
    TestFalse(TEXT("Follow-up combo cannot replace first-attack evidence"), D.Matches(10, 1));
    TestFalse(TEXT("Another first attack cannot replace this attempt"), D.Matches(11, 0));
    D.bWindowOpened = true;
    Expect(TEXT("IncompleteObservation")); // cancellation before close is not a miss
    D.bWindowClosed = true;
    Expect(TEXT("NoContact"));
    D.bContact = true;
    Expect(TEXT("ContactUnresolved")); // overlap without accepted packet is not no-contact
    D.bAccepted = true;
    Expect(TEXT("OtherDefensePolicy")); // no inference about non-parryable or special policy
    D.bEligiblePolicy = true;
    Expect(TEXT("ContactOutsideParryWindow"));
    D.bParryOpenAtContact = true;
    Expect(TEXT("GateMismatch")); // valid normal-policy contact, open window, no Parry
    D.bParryResult = true;
    Expect(TEXT("ParryConfirmed"));
    D.bParryOpenAtContact = false;
    Expect(TEXT("GateMismatch")); // Parry despite a closed sampled window
    D.bEligiblePolicy = false;
    Expect(TEXT("OtherDefensePolicy"));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParryReplayCompletionTest, "Portfolio.Capture.Replay.ParryCompletion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FParryReplayCompletionTest::RunTest(const FString&)
{
    using PortfolioReplay::FParryReplayDiagnostic;
    FParryReplayDiagnostic D;
    TestFalse(TEXT("Empty observation cannot complete"), D.CanComplete(true));
    D.Generation = 10; D.Index = 0; D.bWindowOpened = D.bWindowClosed = true;
    TestFalse(TEXT("No contact cannot complete"), D.CanComplete(true));
    D.RecordContact(10, 0, 100, 7, true);
    TestFalse(TEXT("Pending contact cannot complete"), D.CanComplete(true));
    D.RecordAccepted(10, 0, 100, 7, true, true, true);
    TestEqual(TEXT("Valid contact classified as confirmed"), FString(D.Classify()), FString(TEXT("ParryConfirmed")));
    TestTrue(TEXT("Complete matching Parry may complete"), D.CanComplete(true));
    TestFalse(TEXT("Incomplete binding blocks even matching Parry"), D.CanComplete(false));
    D.bParryOpenAtContact = false;
    TestEqual(TEXT("Closed window plus Parry is mismatch"), FString(D.Classify()), FString(TEXT("GateMismatch")));
    TestFalse(TEXT("Closed-window Parry cannot complete"), D.CanComplete(true));
    D.bParryOpenAtContact = true; D.bParryResult = false;
    TestFalse(TEXT("Open-window non-Parry cannot complete"), D.CanComplete(true));
    D.bParryResult = true; D.bEligiblePolicy = false;
    TestFalse(TEXT("Other defense policy cannot complete"), D.CanComplete(true));
    D.bEligiblePolicy = true; D.Index = 1;
    TestFalse(TEXT("Follow-up index cannot complete first-attack replay"), D.CanComplete(true));
    D.Index = 0; D.Generation = 0;
    TestFalse(TEXT("Unidentified attack cannot complete"), D.CanComplete(true));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FParryReplayRecontactTest, "Portfolio.Capture.Replay.ParryRecontact", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FParryReplayRecontactTest::RunTest(const FString&)
{
    PortfolioReplay::FParryReplayDiagnostic D;
    D.Generation = 10; D.Index = 0;
    TestFalse(TEXT("Accepted without a preceding contact is ignored"), D.RecordAccepted(10, 0, 100, 7, true, true, true));
    TestFalse(TEXT("Contact without an active hit window is ignored"), D.RecordContact(10, 0, 100, INDEX_NONE, false));
    TestTrue(TEXT("First overlap sampled outside parry window"), D.RecordContact(10, 0, 100, 7, false));
    TestFalse(TEXT("Rejected result does not freeze observation"), D.RecordAccepted(10, 0, 100, 7, false, true, false));
    TestEqual(TEXT("Unaccepted overlap remains unresolved"), FString(D.Classify()), FString(TEXT("ContactUnresolved")));
    TestFalse(TEXT("Unrelated generation contact ignored"), D.RecordContact(11, 0, 101, 7, true));
    TestFalse(TEXT("Follow-up combo contact ignored"), D.RecordContact(10, 1, 101, 7, true));
    TestEqual(TEXT("Ignored contact does not move pending frame"), D.ContactFrame, uint64(100));
    TestFalse(TEXT("Later result without recontact is ignored"), D.RecordAccepted(10, 0, 101, 7, true, true, true));
    TestTrue(TEXT("Next-frame valid recontact refreshes pending sample"), D.RecordContact(10, 0, 101, 7, true));
    TestTrue(TEXT("Recontact records new parry state"), D.bParryOpenAtContact);
    TestFalse(TEXT("Old frame result cannot resolve new contact"), D.RecordAccepted(10, 0, 100, 7, true, true, true));
    TestFalse(TEXT("Different hit window result ignored"), D.RecordAccepted(10, 0, 101, 8, true, true, true));
    TestFalse(TEXT("Different generation result ignored"), D.RecordAccepted(11, 0, 101, 7, true, true, true));
    TestFalse(TEXT("Different combo index result ignored"), D.RecordAccepted(10, 1, 101, 7, true, true, true));
    TestTrue(TEXT("Current recontact result accepted"), D.RecordAccepted(10, 0, 101, 7, true, true, true));
    TestTrue(TEXT("Recovered first attack can complete"), D.CanComplete(true));
    TestFalse(TEXT("Duplicate accepted packet cannot overwrite result"), D.RecordAccepted(10, 0, 101, 7, true, true, false));
    TestFalse(TEXT("Post-acceptance overlap cannot overwrite evidence"), D.RecordContact(10, 0, 102, 8, false));
    TestTrue(TEXT("Accepted result remains confirmed"), D.CanComplete(true));
    TestEqual(TEXT("Accepted frame is frozen"), D.ContactFrame, uint64(101));

    PortfolioReplay::FParryReplayDiagnostic NoReceipt;
    NoReceipt.Generation = 20; NoReceipt.Index = 0;
    NoReceipt.RecordContact(20, 0, 200, 9, true);
    // No accepted callback at all, then a new window/frame with a closed parry gate.
    TestTrue(TEXT("Missing callback permits later recontact"), NoReceipt.RecordContact(20, 0, 202, 10, false));
    TestTrue(TEXT("Later valid non-parry result resolves contact"), NoReceipt.RecordAccepted(20, 0, 202, 10, true, true, false));
    TestEqual(TEXT("Latest contact state determines classification"), FString(NoReceipt.Classify()), FString(TEXT("ContactOutsideParryWindow")));
    TestFalse(TEXT("Resolved non-parry does not complete"), NoReceipt.CanComplete(true));
    return true;
}
#endif
