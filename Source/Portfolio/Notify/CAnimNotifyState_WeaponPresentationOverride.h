#pragma once

#include "CoreMinimal.h"
#include "Notify/CAnimNotifyState.h"
#include "CAnimNotifyState_WeaponPresentationOverride.generated.h"

UCLASS(meta = (DisplayName = "Weapon Presentation Override"))
class PORTFOLIO_API UCAnimNotifyState_WeaponPresentationOverride : public UCAnimNotifyState
{
	GENERATED_BODY()

public:
	UCAnimNotifyState_WeaponPresentationOverride();

private:
	// Additive transform in weapon-relative space, blended from identity to this
	// target over the state window. Rotation-only is sufficient for reverse grip.
	UPROPERTY(EditAnywhere, Category = "Weapon Presentation")
	FTransform TargetRelativeOffset = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category = "Weapon Presentation", meta = (ClampMin = "0.0"))
	float BlendInDuration = 0.12f;

	UPROPERTY(EditAnywhere, Category = "Weapon Presentation", meta = (ClampMin = "0.0"))
	float BlendOutDuration = 0.12f;

private:
	struct FRuntimeState
	{
		TWeakObjectPtr<class UCWeaponComponent> WeaponComponent;
		uint32 OverrideHandle = 0;
		float ElapsedTime = 0.f;
		float TotalDuration = 0.f;
	};

	// Notify objects are shared by every mesh that plays their source asset. Runtime
	// state therefore belongs to each mesh instance, never to the notify settings.
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, FRuntimeState> RuntimeStates;

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	float CalculatePresentationAlpha(float InElapsedTime, float InTotalDuration) const;
	static float EaseInOut(float InAlpha);
};
