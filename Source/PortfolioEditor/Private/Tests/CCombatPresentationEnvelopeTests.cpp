#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Type/CCombatPresentationEnvelope.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPresentationEnvelopePhasesTest, "Portfolio.Combat.Presentation.EnvelopePhases",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPresentationEnvelopePhasesTest::RunTest(const FString& Parameters)
{
	FCombatPresentationEnvelope profile;
	profile.Delay = 1.f;
	profile.FadeIn = 2.f;
	profile.Hold = 1.f;
	profile.FadeOut = 2.f;
	TestEqual(TEXT("Before start"), profile.Evaluate(-1.), 0.f);
	TestEqual(TEXT("Delay"), profile.Evaluate(.5), 0.f);
	TestEqual(TEXT("Fade in midpoint"), profile.Evaluate(2.), .5f);
	TestEqual(TEXT("Hold"), profile.Evaluate(3.5), 1.f);
	TestEqual(TEXT("Fade out midpoint"), profile.Evaluate(5.), .5f);
	TestEqual(TEXT("End"), profile.Evaluate(6.), 0.f);
	TestEqual(TEXT("Sustain ignores hold deadline"), profile.Evaluate(100., true), 1.f);
	profile.FadeIn = profile.FadeOut = profile.Hold = profile.Delay = 0.f;
	TestEqual(TEXT("Zero duration finite pulse"), profile.Evaluate(0.), 0.f);
	TestEqual(TEXT("Zero duration sustain"), profile.Evaluate(0., true), 1.f);
	profile.FadeOut = std::numeric_limits<float>::quiet_NaN();
	TestFalse(TEXT("Reject invalid duration"), profile.IsValid());
	TestEqual(TEXT("Invalid profile has no influence"), profile.Evaluate(1.), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPresentationEnvelopeReleaseTest, "Portfolio.Combat.Presentation.EnvelopeRelease",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPresentationEnvelopeReleaseTest::RunTest(const FString& Parameters)
{
	FCombatPresentationEnvelope profile;
	profile.FadeIn = 2.f;
	profile.FadeOut = 2.f;
	FCombatPresentationEnvelopeState state;
	TestTrue(TEXT("Start"), state.Start(profile, 10., true));
	state.Release(11.);
	TestEqual(TEXT("Release does not jump to full strength"), state.Evaluate(11.), .5f);
	TestEqual(TEXT("Fade from captured strength"), state.Evaluate(12.), .25f);
	state.Release(12.);
	TestTrue(TEXT("Repeated release cannot extend lifetime"), state.IsFinished(13.));
	TestEqual(TEXT("Finished value"), state.Evaluate(13.), 0.f);
	TestTrue(TEXT("Restart resets released state"), state.Start(profile, 20., true));
	TestEqual(TEXT("New sustained execution"), state.Evaluate(50.), 1.f);
	TestFalse(TEXT("Invalid start"), state.Start(profile, std::numeric_limits<double>::quiet_NaN()));
	TestTrue(TEXT("Invalid restart clears old state"), state.IsFinished(50.));
	return true;
}
#endif
