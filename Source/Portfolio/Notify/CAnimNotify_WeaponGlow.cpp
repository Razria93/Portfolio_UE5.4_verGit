#include "Notify/CAnimNotify_WeaponGlow.h"
#include "Notify/CCombatNotifyOwnership.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

FString UCAnimNotify_WeaponGlow::GetNotifyName_Implementation() const
{
	return FString(bCounterRush ? TEXT("CounterRush ") : TEXT("PerfectDodge "))
		+ (Cue == EWeaponGlowCue::Start ? TEXT("WeaponGlowStart") : TEXT("WeaponGlowEnd"));
}

void UCAnimNotify_WeaponGlow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	auto* owner = MeshComp ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr;
	if (!owner || owner->GetMesh() != MeshComp) return;
	auto* feedback = owner->FindComponentByClass<UCReactionFeedbackComponent>();
	if (!feedback) return;
	uint64 generation = 0;
	if (bCounterRush)
	{
		const auto* action = owner->FindComponentByClass<UCActionComponent>();
		if (!action || !action->IsActiveActionType(EActionType::CounterRush) || !CCombatNotifyOwnership::AllowsAction(action, EventReference)) return;
		generation = action->GetActiveActionGeneration();
	}
	else
	{
		const auto* reaction = owner->FindComponentByClass<UCReactionComponent>();
		if (!reaction || !reaction->IsActiveReactionType(EReactionType::PerfectDodge) || !CCombatNotifyOwnership::AllowsReaction(reaction, EventReference)) return;
		generation = reaction->GetActiveReactionGeneration();
	}
	if (Cue == EWeaponGlowCue::Start) feedback->StartWeaponGlow(bCounterRush, generation, PeakWeight, FadeIn, FadeOut, SafetyTimeout);
	else feedback->EndWeaponGlow(bCounterRush, generation, FadeOut);
}
