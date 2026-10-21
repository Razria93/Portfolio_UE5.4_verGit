#include "Notify/CAnimNotify.h"

#include "ProjectGlobal.h"
#include "Animation/ActiveMontageInstanceScope.h"

void UCAnimNotify::BranchingPointNotify(FBranchingPointNotifyPayload& InPayload)
{
	FAnimNotifyEventReference reference;
	reference.AddContextData<UE::Anim::FAnimNotifyMontageInstanceContext>(InPayload.MontageInstanceID);
	Notify(InPayload.SkelMeshComponent, InPayload.SequenceAsset, reference);
}
