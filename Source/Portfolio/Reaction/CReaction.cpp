#include "Reaction/CReaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"
#include "Component/CStateComponent.h"

void UCReaction::InitializeReaction(ACharacter* InOwnerCharacter, UCReactionComponent* InOwnerReactionComponent)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	OwnerReactionComponent_Injected = InOwnerReactionComponent;
	check(OwnerReactionComponent_Injected);

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Injected->GetComponentByClass(UCStateComponent::StaticClass()));				// TODO: Refactor Interface
	check(StateComp_Cached);

	ReactionComp_Cached = Cast<UCReactionComponent>(OwnerCharacter_Injected->GetComponentByClass(UCReactionComponent::StaticClass()));		// TODO: Refactor Interface
	check(ReactionComp_Cached);
}

bool UCReaction::PlayReaction(const FReactionData& reactionData)
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!IsValid(OwnerReactionComponent_Injected)) return false;
	if (!IsValid(reactionData.Montage)) return false;

	USkeletalMeshComponent* meshComp = OwnerCharacter_Injected->GetMesh();
	if (!IsValid(meshComp)) return false;

	UAnimInstance* animInstance = meshComp->GetAnimInstance();
	if (!IsValid(animInstance)) return false;

	const float playRate = FMath::Max(0.01f, reactionData.PlayRate);
	animInstance->Montage_Play(reactionData.Montage, playRate);

	return true;
}

void UCReaction::Stop(EReactionStopReason InStopReason, const UCReaction* InNewReaction)
{
	// TODO:
}
