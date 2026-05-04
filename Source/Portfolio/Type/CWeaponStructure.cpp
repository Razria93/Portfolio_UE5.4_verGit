#include "Type/CWeaponStructure.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Component/CMovementComponent.h"
#include "Reaction/CReaction.h"

bool FActionData::IsValidMinimal() const
{
	return IsValid(Montage);
}

void FActionData::BeginPlayMontage(ACharacter* InOwnerCharacter)
{
	UActorComponent* temp = InOwnerCharacter->GetComponentByClass(UCMovementComponent::StaticClass());
	if (!IsValid(temp)) return;

	UCMovementComponent* moveComp = Cast<UCMovementComponent>(temp);
	if (!IsValid(moveComp)) return;

	if (bCanMove == false)
		moveComp->SetStop();

	if (IsValid(Montage))
		InOwnerCharacter->PlayAnimMontage(Montage, PlayRate);
}

void FActionData::EndPlayMontage(ACharacter* InOwnerCharacter)
{
	UActorComponent* temp = InOwnerCharacter->GetComponentByClass(UCMovementComponent::StaticClass());
	if (!IsValid(temp)) return;

	UCMovementComponent* moveComp = Cast<UCMovementComponent>(temp);
	if (!IsValid(moveComp)) return;

	if (bCanMove == false)
		moveComp->SetMove();
}

bool FOverlapContext::IsValidMinimal() const
{
	return IsValid(OwnerActor) && IsValid(DamageCauser) && IsValid(OtherActor);
}

bool FReactionData::IsValidMinimal() const
{
	return ReactionDataKey.ReactionType != EReactionType::None
		&& ReactionDataKey.ReactionType != EReactionType::All
		&& ReactionDataKey.ReactionType != EReactionType::Max
		&& IsValid(ReactionExecutorKey)
		&& IsValid(Montage);
}

bool FReactionQueryContext::IsValidMinimal() const
{
	return IsValid(CurrentReactionExecutor) && IsValid(IncomingReactionExecutor);
}

bool FReactionContext::IsValidMinimal() const
{
	return ReactionData.IsValidMinimal() && IsValid(ReactionExecutor);
}
