#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "System/Combat/CWorldSubsystem_CombatFeedback.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformProcess.h"
#include <limits>

namespace CombatTimeEffectsTest
{
	struct FFixture
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		AActor* Player = World->SpawnActor<AActor>();
		AActor* Attacker = World->SpawnActor<AActor>();
		AActor* Bystander = World->SpawnActor<AActor>();
		UCWorldSubsystem_CombatFeedback* Feedback = World->GetSubsystem<UCWorldSubsystem_CombatFeedback>();

		FFixture()
		{
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Player->CustomTimeDilation = .8f;
			Attacker->CustomTimeDilation = .6f;
		}
		~FFixture()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}
		void HitStop(float InDuration, float InDilation)
		{
			FHitStopRequest request;
			request.SourceActor = Player;
			request.TargetActor = Attacker;
			request.HitStopAudience = EFeedbackAudience::Both;
			request.HitStopDuration = InDuration;
			request.HitStopDilation = InDilation;
			Feedback->RequestHitStop(request);
		}
		void AdvanceGame(float InSeconds)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_All, InSeconds);
			Feedback->Tick(InSeconds);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTimeEffectsOverlapTest, "Portfolio.Combat.TimeEffects.OverlapAndCancellation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTimeEffectsOverlapTest::RunTest(const FString& Parameters)
{
	CombatTimeEffectsTest::FFixture f;
	if (!TestNotNull(TEXT("World owns feedback subsystem"), f.Feedback)) return false;
	const uint64 first = f.Feedback->RequestPerfectDodgeSlow(f.Player, f.Attacker, 60.f, .15f);
	TestTrue(TEXT("Slow receives a nonzero ownership token"), first != 0);
	TestEqual(TEXT("Player slowed"), f.Player->CustomTimeDilation, .15f);
	TestEqual(TEXT("Success attacker slowed"), f.Attacker->CustomTimeDilation, .15f);
	TestEqual(TEXT("Uninvolved actor unchanged"), f.Bystander->CustomTimeDilation, 1.f);
	f.HitStop(.1f, .05f);
	const uint64 second = f.Feedback->RequestPerfectDodgeSlow(f.Player, f.Attacker, 60.f, .2f);
	TestTrue(TEXT("Requests have independent ownership"), first != second);
	TestEqual(TEXT("HitStop wins over both slower effects"), f.Player->CustomTimeDilation, .05f);
	f.Feedback->CancelPerfectDodgeSlow(first);
	TestEqual(TEXT("Cancelling slow does not cancel HitStop"), f.Player->CustomTimeDilation, .05f);
	f.AdvanceGame(.2f);
	TestEqual(TEXT("HitStop expiry reveals remaining slow"), f.Player->CustomTimeDilation, .2f);
	f.Feedback->CancelPerfectDodgeSlow(first);
	TestEqual(TEXT("Stale cancellation cannot remove newer slow"), f.Player->CustomTimeDilation, .2f);
	f.Feedback->CancelPerfectDodgeSlow(second);
	TestEqual(TEXT("Player original non-unit baseline restored"), f.Player->CustomTimeDilation, .8f);
	TestEqual(TEXT("Attacker distinct baseline restored"), f.Attacker->CustomTimeDilation, .6f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTimeEffectsDeadlineTest, "Portfolio.Combat.TimeEffects.IndependentDeadlines",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTimeEffectsDeadlineTest::RunTest(const FString& Parameters)
{
	CombatTimeEffectsTest::FFixture f;
	if (!TestNotNull(TEXT("World owns feedback subsystem"), f.Feedback)) return false;
	f.HitStop(.1f, .05f);
	f.HitStop(.5f, .3f);
	f.AdvanceGame(.2f);
	TestEqual(TEXT("Later weaker HitStop does not extend earlier stronger deadline"), f.Player->CustomTimeDilation, .3f);
	const double gameTime = f.World->GetTimeSeconds();
	f.Feedback->RequestPerfectDodgeSlow(f.Player, f.Attacker, .01f, .1f);
	// Real-time expiry must not depend on world/actor animation advancement.
	FPlatformProcess::Sleep(.03f);
	f.Feedback->Tick(0.f);
	TestEqual(TEXT("World time stayed fixed during real-time wait"), f.World->GetTimeSeconds(), gameTime);
	TestEqual(TEXT("Real-time slow expired while game-time HitStop remains"), f.Player->CustomTimeDilation, .3f);
	f.AdvanceGame(.4f);
	TestEqual(TEXT("Final game-time expiry restores baseline"), f.Player->CustomTimeDilation, .8f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTimeEffectsCleanupTest, "Portfolio.Combat.TimeEffects.ValidationAndCleanup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTimeEffectsCleanupTest::RunTest(const FString& Parameters)
{
	CombatTimeEffectsTest::FFixture f;
	if (!TestNotNull(TEXT("World owns feedback subsystem"), f.Feedback)) return false;
	f.HitStop(-1.f, .1f);
	f.HitStop(1.f, std::numeric_limits<float>::quiet_NaN());
	f.Feedback->RequestPerfectDodgeSlow(f.Player, f.Attacker, 1.f, 2.f);
	TestEqual(TEXT("Invalid requests leave baseline untouched"), f.Player->CustomTimeDilation, .8f);
	f.Feedback->CancelPerfectDodgeSlow(0);
	f.HitStop(60.f, .05f);
	f.Feedback->RequestPerfectDodgeSlow(f.Player, f.Attacker, 60.f, .15f);
	f.World->CleanupWorld();
	TestEqual(TEXT("World cleanup restores player baseline despite pending effects"), f.Player->CustomTimeDilation, .8f);
	TestEqual(TEXT("World cleanup restores attacker baseline"), f.Attacker->CustomTimeDilation, .6f);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatTimeEnvelopeTest, "Portfolio.Combat.TimeEffects.PresentationEnvelope",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCombatTimeEnvelopeTest::RunTest(const FString& Parameters)
{
	CombatTimeEffectsTest::FFixture f;
	FCombatPresentationEnvelope profile;
	profile.Delay = 60.f;
	const uint64 delayed = f.Feedback->RequestPerfectDodgeSlow(f.Player, f.Attacker, profile, .15f);
	TestTrue(TEXT("Valid envelope token"), delayed != 0);
	TestEqual(TEXT("Delay preserves player baseline"), f.Player->CustomTimeDilation, .8f);
	TestEqual(TEXT("Delay preserves attacker baseline"), f.Attacker->CustomTimeDilation, .6f);
	f.Feedback->CancelPerfectDodgeSlow(delayed);
	profile.Delay = 0.f;
	profile.FadeIn = 60.f;
	profile.BlendCurve = NewObject<UCurveFloat>();
	profile.BlendCurve->FloatCurve.AddKey(0.f, .5f);
	profile.BlendCurve->FloatCurve.AddKey(1.f, .5f);
	const uint64 gradual = f.Feedback->RequestPerfectDodgeSlow(f.Player, f.Attacker, profile, .2f);
	f.Feedback->Tick(0.f);
	TestTrue(TEXT("Envelope blends from individual player baseline"), FMath::IsNearlyEqual(f.Player->CustomTimeDilation, .5f));
	TestTrue(TEXT("Envelope blends from individual attacker baseline"), FMath::IsNearlyEqual(f.Attacker->CustomTimeDilation, .4f));
	f.HitStop(60.f, .05f);
	f.Feedback->CancelPerfectDodgeSlow(gradual);
	TestEqual(TEXT("Cancelling interpolated slow preserves HitStop"), f.Player->CustomTimeDilation, .05f);
	// Use an independent actor to prove real-time expiry while the overlapping HitStop stays active.
	profile.BlendCurve = nullptr;
	profile.FadeIn = 0.f;
	profile.Hold = .001f;
	profile.FadeOut = .001f;
	const double gameTime = f.World->GetTimeSeconds();
	f.Feedback->RequestPerfectDodgeSlow(f.Bystander, nullptr, profile, .1f);
	FPlatformProcess::Sleep(.01f);
	f.Feedback->Tick(0.f);
	TestEqual(TEXT("Envelope expires without game-time advancement"), f.Bystander->CustomTimeDilation, 1.f);
	TestEqual(TEXT("Game clock did not advance"), f.World->GetTimeSeconds(), gameTime);
	profile.FadeOut = -1.f;
	TestEqual(TEXT("Invalid envelope returns no lease"), f.Feedback->RequestPerfectDodgeSlow(f.Bystander, nullptr, profile, .1f), uint64(0));
	return true;
}
#endif
