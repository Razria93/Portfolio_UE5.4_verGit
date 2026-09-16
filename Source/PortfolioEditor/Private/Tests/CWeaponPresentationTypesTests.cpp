#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Type/CWeaponPresentationTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponPivotTargetOrientationTest,
	"Portfolio.Weapon.Presentation.TargetOrientation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWeaponPivotTargetOrientationTest::RunTest(const FString& Parameters)
{
	FWeaponPivotRotationSpec spec;
	spec.Mode = EWeaponPivotRotationMode::TargetOrientation;
	spec.TargetOrientation = FRotator(0.f, 90.f, 0.f);

	const FQuat expected = spec.TargetOrientation.Quaternion().GetNormalized();
	TestTrue(TEXT("Target orientation reaches the authored rotation at alpha one."), spec.Evaluate(FQuat::Identity, 1.f).Equals(expected, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Target orientation preserves the source at alpha zero."), spec.Evaluate(FQuat::Identity, 0.f).Equals(FQuat::Identity, KINDA_SMALL_NUMBER));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWeaponPivotSignedAxisAngleTest,
	"Portfolio.Weapon.Presentation.SignedAxisAngle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWeaponPivotSignedAxisAngleTest::RunTest(const FString& Parameters)
{
	FWeaponPivotRotationSpec positiveTurn;
	positiveTurn.Mode = EWeaponPivotRotationMode::SignedLocalAxisAngle;
	positiveTurn.LocalAxis = FVector::UpVector;
	positiveTurn.SignedAngleDegrees = 360.f;

	FWeaponPivotRotationSpec negativeTurn = positiveTurn;
	negativeTurn.SignedAngleDegrees = -360.f;

	const FQuat positiveQuarterTurn(FVector::UpVector, HALF_PI);
	const FQuat negativeQuarterTurn(FVector::UpVector, -HALF_PI);
	TestTrue(TEXT("Positive winding is preserved before the final equivalent orientation."), positiveTurn.Evaluate(FQuat::Identity, 0.25f).Equals(positiveQuarterTurn, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("Negative winding is preserved before the final equivalent orientation."), negativeTurn.Evaluate(FQuat::Identity, 0.25f).Equals(negativeQuarterTurn, KINDA_SMALL_NUMBER));

	FWeaponPivotRotationSpec doubleTurn = positiveTurn;
	doubleTurn.SignedAngleDegrees = 720.f;
	const FQuat expectedQuarterOfDoubleTurn(FVector::UpVector, PI);
	TestTrue(TEXT("A 720 degree request retains its intermediate 180 degree orientation at quarter progress."), doubleTurn.Evaluate(FQuat::Identity, 0.25f).Equals(expectedQuarterOfDoubleTurn, KINDA_SMALL_NUMBER));

	FWeaponPivotRotationSpec invalidAxis = positiveTurn;
	invalidAxis.LocalAxis = FVector::ZeroVector;
	TestTrue(TEXT("A zero-length signed axis is rejected."), !invalidAxis.IsValid());
	return true;
}

#endif
