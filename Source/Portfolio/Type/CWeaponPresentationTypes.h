#pragma once

#include "CoreMinimal.h"
#include "CWeaponPresentationTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponPivotRotationMode : uint8
{
	TargetOrientation,
	SignedLocalAxisAngle,

	Max UMETA(Hidden),
};

USTRUCT(BlueprintType)
struct FWeaponPivotRotationSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation")
	EWeaponPivotRotationMode Mode = EWeaponPivotRotationMode::TargetOrientation;

	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation", meta = (EditCondition = "Mode == EWeaponPivotRotationMode::TargetOrientation", EditConditionHides))
	FRotator TargetOrientation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation", meta = (EditCondition = "Mode == EWeaponPivotRotationMode::SignedLocalAxisAngle", EditConditionHides))
	FVector LocalAxis = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation", meta = (EditCondition = "Mode == EWeaponPivotRotationMode::SignedLocalAxisAngle", EditConditionHides))
	float SignedAngleDegrees = 0.f;

public:
	bool IsValid() const
	{
		switch (Mode)
		{
		case EWeaponPivotRotationMode::TargetOrientation:
			return !TargetOrientation.ContainsNaN();

		case EWeaponPivotRotationMode::SignedLocalAxisAngle:
			return !LocalAxis.ContainsNaN()
				&& !LocalAxis.IsNearlyZero()
				&& FMath::IsFinite(SignedAngleDegrees);

		default:
			return false;
		}
	}

	FQuat Evaluate(const FQuat& InSourceRotation, float InAlpha) const
	{
		FQuat sourceRotation = InSourceRotation;
		sourceRotation.Normalize();

		const float alpha = FMath::Clamp(InAlpha, 0.f, 1.f);
		switch (Mode)
		{
		case EWeaponPivotRotationMode::TargetOrientation:
		{
			FQuat targetRotation = TargetOrientation.Quaternion();
			targetRotation.Normalize();
			return FQuat::Slerp(sourceRotation, targetRotation, alpha).GetNormalized();
		}

		case EWeaponPivotRotationMode::SignedLocalAxisAngle:
		{
			const FVector axis = LocalAxis.GetSafeNormal();
			if (axis.IsNearlyZero()) return sourceRotation;

			const float angleRadians = FMath::DegreesToRadians(SignedAngleDegrees * alpha);
			return (sourceRotation * FQuat(axis, angleRadians)).GetNormalized();
		}

		default:
			return sourceRotation;
		}
	}
};
