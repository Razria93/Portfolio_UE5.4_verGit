#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Core/Debug/FCaptureCamera.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCaptureCameraFramingTest, "Portfolio.Camera.CaptureFraming",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCaptureCameraFramingTest::RunTest(const FString& Parameters)
{
	FMinimalViewInfo baseline;
	baseline.Location = FVector(-300.f, 0.f, 60.f);
	baseline.Rotation = FRotator(-5.f, 0.f, 0.f);
	baseline.FOV = 83.f;
	FCaptureCameraState state;
	FCaptureCameraSettings settings;
	FMinimalViewInfo view = baseline;
	const FVector player(0.f, 0.f, 0.f);
	const FVector pivot(0.f, 0.f, 55.f);
	const TOptional<FVector> target(FVector(200.f, 0.f, 0.f));
	FCaptureCamera::Apply(settings, state, player, pivot, target, 1.f / 60.f, view);
	TestTrue(TEXT("Disabled mode preserves location exactly"), view.Location == baseline.Location);
	TestTrue(TEXT("Disabled mode preserves rotation exactly"), view.Rotation == baseline.Rotation);
	settings.bEnabled = true;
	settings.InterpSpeed = 0.f;
	FCaptureCamera::Apply(settings, state, player, pivot, target, 1.f / 60.f, view);
	TestTrue(TEXT("Capture separates the camera from the attack axis"), FMath::Abs(view.Location.Y) > 100.f);
	const FVector focus(100.f, 0.f, 35.f);
	TestTrue(TEXT("Lock-on frames the midpoint"), FVector::DotProduct(view.Rotation.Vector(), (focus - view.Location).GetSafeNormal()) > 0.9999f);
	TestEqual(TEXT("FOV remains unchanged"), view.FOV, baseline.FOV);
	const FVector oldOffset = state.LocationOffset;
	settings.bEnabled = false;
	settings.InterpSpeed = 6.f;
	view = baseline;
	FCaptureCamera::Apply(settings, state, player, pivot, target, 1.f / 60.f, view);
	TestTrue(TEXT("Disable blends back rather than snapping"), state.LocationOffset.Size() < oldOffset.Size() && !state.LocationOffset.IsNearlyZero());
	for (int32 i = 0; i < 240; ++i)
	{
		view = baseline;
		FCaptureCamera::Apply(settings, state, player, pivot, target, 1.f / 60.f, view);
	}
	TestFalse(TEXT("Restore completes"), state.IsTransitioning());
	TestTrue(TEXT("Original view is restored exactly"), view.Location == baseline.Location && view.Rotation == baseline.Rotation);
	settings.bEnabled = true;
	settings.InterpSpeed = 0.f;
	settings.YawOffset = 0.f;
	settings.SideOffset = -150.f;
	view = baseline;
	FCaptureCamera::Apply(settings, state, player, pivot, TOptional<FVector>(), 1.f / 60.f, view);
	TestTrue(TEXT("Opposite shoulder works without a target"), view.Location.Y < -100.f);
	TestTrue(TEXT("Free look keeps the base rotation when orbit is zero"), view.Rotation.Equals(baseline.Rotation));
	// Large target weight must clamp to the target, rather than aiming past it.
	settings.TargetWeight = 8.f;
	view = baseline;
	FCaptureCamera::Apply(settings, state, player, pivot, target, 1.f / 60.f, view);
	TestTrue(TEXT("Target weight is clamped"), FVector::DotProduct(view.Rotation.Vector(), (target.GetValue() + FVector(0,0,35) - view.Location).GetSafeNormal()) > 0.9999f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCaptureCameraCollisionTest, "Portfolio.Camera.CaptureCollision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCaptureCameraCollisionTest::RunTest(const FString& Parameters)
{
	UWorld* world = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* wall = world->SpawnActor<AActor>();
	UBoxComponent* box = NewObject<UBoxComponent>(wall);
	wall->SetRootComponent(box);
	box->SetBoxExtent(FVector(10.f, 1000.f, 1000.f));
	box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	box->SetCollisionResponseToAllChannels(ECR_Ignore);
	box->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	box->RegisterComponent();
	wall->SetActorLocation(FVector(-200.f, 0.f, 55.f));
	FCaptureCameraSettings settings;
	settings.bEnabled = true;
	settings.InterpSpeed = 0.f;
	settings.SideOffset = 0.f;
	settings.YawOffset = 0.f;
	FCaptureCameraState state;
	FMinimalViewInfo view;
	view.Location = FVector(-300.f, 0.f, 60.f);
	view.Rotation = FRotator::ZeroRotator;
	FCaptureCamera::Apply(settings, state, FVector::ZeroVector, FVector(0.f,0.f,55.f),
		TOptional<FVector>(FVector(200.f,0.f,0.f)), 1.f/60.f, view, world, nullptr, true);
	TestTrue(TEXT("Camera sphere stops before the wall"), view.Location.X > -190.f && view.Location.X < -100.f);
	TestTrue(TEXT("Collision-adjusted camera still looks at the focus"), FVector::DotProduct(view.Rotation.Vector(),
		(FVector(100.f,0.f,35.f)-view.Location).GetSafeNormal()) > 0.9999f);
	world->DestroyWorld(false);
	return true;
}
#endif
