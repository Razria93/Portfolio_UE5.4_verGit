#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraTypes.h"

class UWorld;
class AActor;

// View-only framing. Never writes pawn/controller rotation or SpringArm settings.
struct PORTFOLIO_API FCaptureCameraSettings
{
	bool bEnabled = false;
	float SideOffset = 100.f;
	float DistanceOffset = 100.f;
	float HeightOffset = 35.f;
	float YawOffset = -15.f;
	float TargetWeight = 0.5f;
	float FocusHeight = 35.f;
	float InterpSpeed = 6.f;
};

struct PORTFOLIO_API FCaptureCameraState
{
	FVector LocationOffset = FVector::ZeroVector;
	FRotator RotationOffset = FRotator::ZeroRotator;

	bool IsTransitioning() const;
};

class PORTFOLIO_API FCaptureCamera
{
public:
	static FCaptureCameraSettings ReadSettings();
	static void Apply(const FCaptureCameraSettings& Settings, FCaptureCameraState& State,
		const FVector& PlayerLocation, const FVector& Pivot, const TOptional<FVector>& TargetLocation,
		float DeltaTime, FMinimalViewInfo& InOutView, UWorld* World = nullptr,
		const AActor* IgnoredActor = nullptr, bool bCollisionTest = false,
		float ProbeSize = 12.f, ECollisionChannel ProbeChannel = ECC_Camera);
};
