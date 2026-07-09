#pragma once

#include "CoreMinimal.h"
#include "CWorldSubSystemStructure.generated.h"

UENUM(BlueprintType)
enum class ECombatRole : uint8
{
	None,
	Engage,
	Alert
};

UENUM(BlueprintType)
enum class EAIUpdatePrecision : uint8
{
	High,
	Reduced,
	Low
};

UENUM(BlueprintType)
enum class EFeedbackAudience : uint8
{
	None,
	Source,
	Target,
	Both
};

USTRUCT(BlueprintType)
struct FEngageRequestContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	class ACAIController* RequestController = nullptr;

	UPROPERTY(Transient)
	class AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	int TargetPriority = INT_MAX;

	UPROPERTY(Transient)
	float DistanceToTarget = 0.f;

	UPROPERTY(Transient)
	bool bWasEngaged = false;

public:
	FEngageRequestContext() = default;
};

USTRUCT(BlueprintType)
struct FEngageAssignmentContext
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	AActor* TargetActor = nullptr;

	UPROPERTY(Transient)
	ECombatRole CombatRole = ECombatRole::None;

public:
	FEngageAssignmentContext() = default;

public:
	bool IsValidAssignment() const
	{
		return IsValid(TargetActor) && CombatRole != ECombatRole::None;
	}
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
	class AActor* SourceActor = nullptr;

	UPROPERTY(EditAnywhere)
	class AActor* TargetActor = nullptr;
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
	class AActor* SourceActor = nullptr;

	UPROPERTY(EditAnywhere)
	class AActor* TargetActor = nullptr;

	UPROPERTY(EditAnywhere)
	FVector EventLocation = FVector::ZeroVector;
};
