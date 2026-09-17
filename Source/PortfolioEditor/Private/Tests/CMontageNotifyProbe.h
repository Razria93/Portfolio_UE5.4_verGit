#pragma once
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimInstance.h"
#include "CMontageNotifyProbe.generated.h"

// Transient automation probe; never saved into project assets.
UCLASS()
class UCMontageNotifyProbe : public UAnimNotify
{
	GENERATED_BODY()
public:
	int32 Deliveries = 0;
	virtual void Notify(USkeletalMeshComponent*, UAnimSequenceBase*, const FAnimNotifyEventReference&) override { ++Deliveries; }
	virtual void BranchingPointNotify(FBranchingPointNotifyPayload&) override { ++Deliveries; }
};

UCLASS(Transient)
class UCMontageProbeAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	void AdvanceProbe(float DeltaTime) { Montage_Advance(DeltaTime); TriggerAnimNotifies(DeltaTime); }
};
