#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"

class AActor;

#include "CCombatFeedbackTypes.generated.h"

UENUM(BlueprintType)
enum class EFeedbackAudience : uint8
{
	None,
	Source,
	Target,
	Both
};

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
