#include "Notify/CAnimNotify_BodyGlow.h"
#include "Notify/CCombatNotifyOwnership.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Reaction/CReaction.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

FString UCAnimNotify_BodyGlow::GetNotifyName_Implementation() const
{
	return bStart ? TEXT("Body Glow Start") : TEXT("Body Glow End");
}

void UCAnimNotify_BodyGlow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	auto* owner = MeshComp ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr;
	if (!owner || owner->GetMesh() != MeshComp) return;
	const auto* reaction = owner->FindComponentByClass<UCReactionComponent>();
	auto* feedback = owner->FindComponentByClass<UCReactionFeedbackComponent>();
	if (!reaction || !feedback || !reaction->IsActiveReactionType(EReactionType::PerfectDodge)
		|| !CCombatNotifyOwnership::AllowsReaction(reaction, EventReference)) return;
	const auto* executor = reaction->GetActiveReactionExecutor();
	if (!IsValid(executor)) return;
	const uint64 generation = reaction->GetActiveReactionGeneration();
	const int32 instance = executor->GetActiveMontageInstanceId();
	if (bStart) feedback->StartBodyGlow(generation, instance, PeakWeight, FadeIn, FadeOut, SafetyTimeout);
	else feedback->EndBodyGlow(generation, instance, FadeOut);
}
