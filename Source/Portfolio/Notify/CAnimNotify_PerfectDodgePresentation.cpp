#include "Notify/CAnimNotify_PerfectDodgePresentation.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UCAnimNotify_PerfectDodgePresentation::UCAnimNotify_PerfectDodgePresentation()
{
	TriggerReactionType = EReactionType::PerfectDodge;
}

FString UCAnimNotify_PerfectDodgePresentation::GetNotifyName_Implementation() const
{
	return Cue == EPerfectDodgePresentationCue::Slow ? TEXT("PerfectDodge Slow (Start)") : TEXT("PerfectDodge Vignette (Start)");
}

void UCAnimNotify_PerfectDodgePresentation::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	UCReactionComponent* reaction = GetReactionComponent(MeshComp);
	if (!CanProcessReactionNotify(reaction, EventReference) || !reaction->IsActiveReactionType(EReactionType::PerfectDodge)) return;
	auto* feedback = reaction->GetOwner()->FindComponentByClass<UCReactionFeedbackComponent>();
	if (!feedback) return;
	switch (Cue)
	{
	case EPerfectDodgePresentationCue::Slow:
		feedback->StartPerfectDodgeSlow(Envelope, SlowDilation, bAffectPlayer, bAffectAttacker);
		break;
	case EPerfectDodgePresentationCue::Vignette:
		feedback->StartPerfectDodgeVignette(Envelope, VignetteIntensity);
		break;
	}
}
