#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Type/CCounterRushTypes.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCounterRushPlanTest, "Portfolio.Combat.CounterRush.CapsulePlan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCounterRushPlanTest::RunTest(const FString& Parameters)
{
	FCounterRushTuning tuning;
	tuning.ExtraGap = 20.f;
	tuning.MaxTravelDistance = 500.f;
	const FVector origin(0, 0, 90);
	auto plan = FCounterRushPlan::Build(origin, FVector(300,0,200), 40.f, 60.f, tuning);
	TestTrue(TEXT("Valid plan"), plan.bValid);
	TestEqual(TEXT("Capsule radii plus gap"), plan.CenterSpacing, 120.f);
	TestEqual(TEXT("Travel subtracts center spacing"), plan.TravelDistance, 180.f);
	TestTrue(TEXT("Vertical position preserved"), plan.LimitedEnd.Equals(FVector(180,0,90)));
	tuning.MaxTravelDistance = 75.f;
	plan = FCounterRushPlan::Build(origin, FVector(300,0,200), 40.f, 60.f, tuning);
	TestEqual(TEXT("Travel cap"), plan.TravelDistance, 75.f);
	TestTrue(TEXT("Desired and limited endpoints remain distinct"), plan.DesiredEnd.Equals(FVector(180,0,90)) && plan.LimitedEnd.Equals(FVector(75,0,90)));
	plan = FCounterRushPlan::Build(origin, FVector(50,0,90), 40.f, 60.f, tuning);
	TestEqual(TEXT("Close target never moves backwards"), plan.TravelDistance, 0.f);
	TestTrue(TEXT("Close target keeps current location"), plan.LimitedEnd.Equals(origin));
	plan = FCounterRushPlan::Build(origin, origin, 40.f, 60.f, tuning);
	TestTrue(TEXT("Coincident centers remain finite"), plan.bValid && plan.Direction.IsZero() && plan.TravelDistance == 0.f);
	tuning.MaxTravelDistance = 0.f;
	plan = FCounterRushPlan::Build(origin, FVector(1000,0,90), 40.f, 60.f, tuning);
	TestEqual(TEXT("Zero cap valid stationary attack"), plan.TravelDistance, 0.f);
	tuning.ExtraGap = -1.f;
	TestFalse(TEXT("Invalid gap rejected"), FCounterRushPlan::Build(origin, origin, 40.f, 60.f, tuning).bValid);
	tuning.ExtraGap = 0.f;
	TestFalse(TEXT("NaN radius rejected"), FCounterRushPlan::Build(origin, origin, std::numeric_limits<float>::quiet_NaN(), 60.f, tuning).bValid);
	tuning.Duration = 0.f;
	TestFalse(TEXT("Zero duration rejected"), tuning.IsValid());
	return true;
}
#endif
