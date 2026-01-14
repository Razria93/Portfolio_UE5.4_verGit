#include "Reaction/CReaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CStateComponent.h"
#include "Component/CReactionComponent.h"

void UCReaction::InitializeReaction(ACharacter* InOwnerCharacter, const TArray<FReactionData> InReactionDatas)
{
	OwnerCharacter_Injected = InOwnerCharacter;
	check(OwnerCharacter_Injected);

	ReactionDatas_Injected = InReactionDatas;

	StateComp_Cached = Cast<UCStateComponent>(OwnerCharacter_Injected->GetComponentByClass(UCStateComponent::StaticClass()));						// TODO: Refactor Interface
	check(StateComp_Cached);

	ReactionComp_Cached = Cast<UCReactionComponent>(OwnerCharacter_Injected->GetComponentByClass(UCReactionComponent::StaticClass()));						// TODO: Refactor Interface
	check(ReactionComp_Cached);
}

void UCReaction::PlayReaction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bIsReaction = true;

	StateComp_Cached->SetReactionMode();

	// NOTE: To be implemented detail by derived classes
}

void UCReaction::BeginPlayReaction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bBeginReaction = true;

	// NOTE: To be implemented detail by derived classes
}

void UCReaction::EndPlayReaction()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(StateComp_Cached)) return;

	bIsReaction = false;
	bBeginReaction = false;

	StateComp_Cached->SetIdleMode();

	// NOTE: To be implemented detail by derived classes
}
