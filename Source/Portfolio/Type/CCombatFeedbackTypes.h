#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"

class AActor;

#include "CCombatFeedbackTypes.generated.h"

// Enum

UENUM(BlueprintType)
enum class EFeedbackAudience : uint8
{
	None,
	Source,
	Target,
	Both
};

// Data / Config

USTRUCT(BlueprintType)
struct FHitStopFeedbackTuning
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "HitStop")
	EFeedbackAudience Audience = EFeedbackAudience::Both;

	UPROPERTY(EditAnywhere, Category = "HitStop")
	float Duration = 0.04f;

	UPROPERTY(EditAnywhere, Category = "HitStop")
	float Dilation = 0.05f;
};

USTRUCT(BlueprintType)
struct FHitCameraShakeFeedbackTuning
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	EFeedbackAudience Audience = EFeedbackAudience::Both;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	float BaseScale = 1.f;
};

USTRUCT(BlueprintType)
struct FPlayerCameraShakeFeedbackTuning
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	float LocalTargetScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	float LocalSourceScale = 0.5f;
};

// Request

USTRUCT(BlueprintType)
struct FHitStopRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float HitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere)
	float HitStopDilation = 0.05f;

	UPROPERTY(EditAnywhere)
	EFeedbackAudience HitStopAudience = EFeedbackAudience::Both;

	UPROPERTY(EditAnywhere)
	AActor* SourceActor = nullptr;

	UPROPERTY(EditAnywhere)
	AActor* TargetActor = nullptr;
};

USTRUCT(BlueprintType)
struct FCameraShakeRequest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShakeBase> CameraShakeClass = nullptr;

	UPROPERTY(EditAnywhere)
	float CameraShakeBaseScale = 1.f;

	UPROPERTY(EditAnywhere)
	EFeedbackAudience CameraShakeAudience = EFeedbackAudience::Both;

	UPROPERTY(EditAnywhere)
	AActor* SourceActor = nullptr;

	UPROPERTY(EditAnywhere)
	AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere)
	FVector EventLocation = FVector::ZeroVector;
};
