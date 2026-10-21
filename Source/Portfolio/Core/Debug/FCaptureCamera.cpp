#include "Core/Debug/FCaptureCamera.h"

#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"

namespace
{
#if !UE_BUILD_SHIPPING
	TAutoConsoleVariable<int32> CVarCaptureEnabled(TEXT("Portfolio.CaptureCamera.Enabled"), 0,
		TEXT("View-only capture framing. 0 restores the gameplay camera; 1 enables capture framing."), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarCaptureSide(TEXT("Portfolio.CaptureCamera.SideOffset"), 100.f,
		TEXT("Lateral camera offset in cm. Negative selects the opposite shoulder. [-300, 300]"), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarCaptureDistance(TEXT("Portfolio.CaptureCamera.DistanceOffset"), 100.f,
		TEXT("Additional horizontal camera distance in cm. [-100, 600]"), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarCaptureHeight(TEXT("Portfolio.CaptureCamera.HeightOffset"), 35.f,
		TEXT("Additional camera height in cm. [-100, 300]"), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarCaptureYaw(TEXT("Portfolio.CaptureCamera.YawOffset"), -15.f,
		TEXT("Camera orbit offset in degrees; does not rotate the pawn. [-60, 60]"), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarCaptureTargetWeight(TEXT("Portfolio.CaptureCamera.TargetWeight"), 0.5f,
		TEXT("Lock-on framing focus: 0 = player, 0.5 = midpoint, 1 = target."), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarCaptureFocusHeight(TEXT("Portfolio.CaptureCamera.FocusHeight"), 35.f,
		TEXT("Focus height above actor origins (capsule centers), in cm. [-100, 200]"), ECVF_Cheat);
	TAutoConsoleVariable<float> CVarCaptureInterp(TEXT("Portfolio.CaptureCamera.InterpSpeed"), 6.f,
		TEXT("Framing interpolation speed; 0 applies immediately. [0, 30]"), ECVF_Cheat);
#endif
}

bool FCaptureCameraState::IsTransitioning() const
{
	return !LocationOffset.IsNearlyZero(0.01f) || !RotationOffset.IsNearlyZero(0.01f);
}

FCaptureCameraSettings FCaptureCamera::ReadSettings()
{
	FCaptureCameraSettings result;
#if !UE_BUILD_SHIPPING
	result.bEnabled = CVarCaptureEnabled.GetValueOnGameThread() != 0;
	result.SideOffset = CVarCaptureSide.GetValueOnGameThread();
	result.DistanceOffset = CVarCaptureDistance.GetValueOnGameThread();
	result.HeightOffset = CVarCaptureHeight.GetValueOnGameThread();
	result.YawOffset = CVarCaptureYaw.GetValueOnGameThread();
	result.TargetWeight = CVarCaptureTargetWeight.GetValueOnGameThread();
	result.FocusHeight = CVarCaptureFocusHeight.GetValueOnGameThread();
	result.InterpSpeed = CVarCaptureInterp.GetValueOnGameThread();
#endif
	return result;
}

void FCaptureCamera::Apply(const FCaptureCameraSettings& Settings, FCaptureCameraState& State,
	const FVector& PlayerLocation, const FVector& Pivot, const TOptional<FVector>& TargetLocation,
	float DeltaTime, FMinimalViewInfo& InOutView, UWorld* World, const AActor* IgnoredActor,
	bool bCollisionTest, float ProbeSize, ECollisionChannel ProbeChannel)
{
	if (!Settings.bEnabled && !State.IsTransitioning())
	{
		State = FCaptureCameraState();
		return;
	}

	const FVector baseLocation = InOutView.Location;
	const FRotator baseRotation = InOutView.Rotation;
	const float speed = FMath::Clamp(Settings.InterpSpeed, 0.f, 30.f);
	const float delta = FMath::Max(DeltaTime, 0.f);
	FVector desiredOffset = FVector::ZeroVector;
	if (Settings.bEnabled)
	{
		const FRotator orbit(0.f, baseRotation.Yaw + FMath::Clamp(Settings.YawOffset, -60.f, 60.f), 0.f);
		const float distance = FMath::Clamp(static_cast<float>((baseLocation - Pivot).Size2D())
			+ FMath::Clamp(Settings.DistanceOffset, -100.f, 600.f), 80.f, 1500.f);
		FVector desiredLocation = Pivot - orbit.Vector() * distance
			+ orbit.RotateVector(FVector::RightVector) * FMath::Clamp(Settings.SideOffset, -300.f, 300.f);
		desiredLocation.Z = baseLocation.Z + FMath::Clamp(Settings.HeightOffset, -100.f, 300.f);
		desiredOffset = desiredLocation - baseLocation;
	}

	State.LocationOffset = speed <= 0.f ? desiredOffset : FMath::VInterpTo(State.LocationOffset, desiredOffset, delta, speed);
	InOutView.Location = baseLocation + State.LocationOffset;
	// Sweep the final interpolated position, so blending cannot move the camera through a wall.
	if (bCollisionTest && IsValid(World))
	{
		FHitResult hit;
		FCollisionQueryParams params(SCENE_QUERY_STAT(CaptureCamera), false, IgnoredActor);
		if (World->SweepSingleByChannel(hit, Pivot, InOutView.Location, FQuat::Identity, ProbeChannel,
			FCollisionShape::MakeSphere(FMath::Max(ProbeSize, 1.f)), params))
		{
			InOutView.Location = hit.bStartPenetrating ? baseLocation : hit.Location;
		}
	}

	FRotator desiredRotationOffset = FRotator::ZeroRotator;
	if (Settings.bEnabled)
	{
		FRotator desiredRotation = baseRotation;
		if (TargetLocation.IsSet())
		{
			const FVector focus = FMath::Lerp(PlayerLocation, TargetLocation.GetValue(), FMath::Clamp(Settings.TargetWeight, 0.f, 1.f))
				+ FVector(0.f, 0.f, FMath::Clamp(Settings.FocusHeight, -100.f, 200.f));
			if (!(focus - InOutView.Location).IsNearlyZero()) desiredRotation = (focus - InOutView.Location).Rotation();
		}
		else
		{
			desiredRotation.Yaw += FMath::Clamp(Settings.YawOffset, -60.f, 60.f);
		}
		desiredRotationOffset = (desiredRotation - baseRotation).GetNormalized();
	}
	State.RotationOffset = speed <= 0.f ? desiredRotationOffset : FMath::RInterpTo(State.RotationOffset, desiredRotationOffset, delta, speed);
	InOutView.Rotation = (baseRotation + State.RotationOffset).GetNormalized();
	if (!Settings.bEnabled && !State.IsTransitioning())
	{
		State = FCaptureCameraState();
		InOutView.Location = baseLocation;
		InOutView.Rotation = baseRotation;
	}
}
