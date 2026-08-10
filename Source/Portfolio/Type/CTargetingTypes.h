#pragma once

#include "CoreMinimal.h"
#include "CTargetingTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class ETargetSwitchDirection : uint8
{
	Left,
	Right,

	Max,
};

USTRUCT(BlueprintType)
struct FTargetingTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0"))
	float MaxTargetDistance = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxTargetAngleDegrees = 55.f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0"))
	float DistanceScoreWeight = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.0"))
	float AngleScoreWeight = 0.65f;

	UPROPERTY(EditAnywhere, Category = "Targeting", meta = (ClampMin = "0.01"))
	float ValidationInterval = 0.1f;
};

USTRUCT(BlueprintType)
struct FTargetLockAssistTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Targeting|LockAssist", meta = (ClampMin = "0.0"))
	float CameraRotationInterpSpeed = 8.f;

	UPROPERTY(EditAnywhere, Category = "Targeting|LockAssist")
	FVector TargetFocusOffset = FVector(0.f, 0.f, 0.f);
};

USTRUCT(BlueprintType)
struct FTargetMarkerTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Targeting|Marker")
	int32 WidgetZOrder = 10;
};

USTRUCT(BlueprintType)
struct FTargetMarkerViewData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Target HUD|Marker")
	bool bVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Target HUD|Marker")
	FVector2D WidgetPosition = FVector2D::ZeroVector;
};

struct FTargetingDebugSnapshot
{
	TWeakObjectPtr<AActor> TargetActor;
	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewForward = FVector::ForwardVector;
	FVector TargetLocation = FVector::ZeroVector;
	float Distance = 0.f;
	float MaxTargetDistance = 0.f;
	float Dot = 0.f;
	float MinDot = 0.f;
	float AngleScore = 0.f;
	float DistanceScore = 0.f;
	float FinalScore = 0.f;
	bool bWithinRange = false;
	bool bWithinViewCone = false;
};
