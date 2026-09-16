#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState_ActionBase.h"
#include "Type/CWeaponPresentationTypes.h"
#include "CAnimNotifyState_WeaponPivotRotationTransition.generated.h"

UCLASS(meta = (DisplayName = "Weapon Pivot Rotation Transition"))
class PORTFOLIO_API UCAnimNotifyState_WeaponPivotRotationTransition : public UCAnimNotifyState_ActionBase
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation")
	EWeaponPivotRotationMode RotationMode = EWeaponPivotRotationMode::TargetOrientation;

	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation", meta = (EditCondition = "RotationMode == EWeaponPivotRotationMode::TargetOrientation", EditConditionHides))
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation", meta = (EditCondition = "RotationMode == EWeaponPivotRotationMode::SignedLocalAxisAngle", EditConditionHides))
	FVector LocalAxis = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation", meta = (EditCondition = "RotationMode == EWeaponPivotRotationMode::SignedLocalAxisAngle", EditConditionHides))
	float SignedAngleDegrees = 0.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Pivot Rotation", meta = (ClampMin = "0.01"))
	float BlendExponent = 2.f;

private:
	struct FRuntimeState
	{
		TWeakObjectPtr<class UCWeaponComponent> WeaponComponent;
		uint32 TransitionHandle = 0;
		float ElapsedTime = 0.f;
		float TotalDuration = 0.f;
	};

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FRuntimeState> RuntimeStates;

protected:
	virtual FString GetNotifyName_Implementation() const override;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	bool IsConfigurationValid() const;

protected:
	bool BeginTransition(class UCWeaponComponent* InWeaponComp, uint32& OutTransitionHandle) const;
	bool UpdateTransition(class UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle, float InAlpha) const;
	bool CompleteTransition(class UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const;
	void CancelTransition(class UCWeaponComponent* InWeaponComp, uint32 InTransitionHandle) const;

private:
	FWeaponPivotRotationSpec BuildRotationSpec() const;
	float CalculateAlpha(float InElapsedTime, float InTotalDuration) const;
};
