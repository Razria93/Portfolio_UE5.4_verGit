#include "Notify/CAnimNotifyState.h"

#include "ProjectGlobal.h"
#include "Animation/ActiveMontageInstanceScope.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifyEndDataContext.h"

void UCAnimNotifyState::BranchingPointNotifyBegin(FBranchingPointNotifyPayload& InPayload)
{
	FAnimNotifyEventReference reference;
	reference.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(InPayload.MontageInstanceID);
	NotifyBegin(InPayload.SkelMeshComponent, InPayload.SequenceAsset,
		InPayload.NotifyEvent ? InPayload.NotifyEvent->GetDuration() : 0.f, reference);
}

void UCAnimNotifyState::BranchingPointNotifyTick(FBranchingPointNotifyPayload& InPayload, float InFrameDeltaTime)
{
	FAnimNotifyEventReference reference;
	reference.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(InPayload.MontageInstanceID);
	NotifyTick(InPayload.SkelMeshComponent, InPayload.SequenceAsset, InFrameDeltaTime, reference);
}

void UCAnimNotifyState::BranchingPointNotifyEnd(FBranchingPointNotifyPayload& InPayload)
{
	FAnimNotifyEventReference reference;
	reference.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(InPayload.MontageInstanceID);
	if (InPayload.bReachedEnd) reference.AddContextData<UE::Anim::FAnimNotifyEndDataContext>(true);
	NotifyEnd(InPayload.SkelMeshComponent, InPayload.SequenceAsset, reference);
}
