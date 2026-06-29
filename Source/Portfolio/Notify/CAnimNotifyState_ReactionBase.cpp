#include "Notify/CAnimNotifyState_ReactionBase.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"

UCAnimNotifyState_ReactionBase::UCAnimNotifyState_ReactionBase()
{
}

bool UCAnimNotifyState_ReactionBase::CanProcessReactionNotify(const UCReactionComponent* InReactionComp) const
{
	if (!IsValid(InReactionComp)) return false;
	if (!InReactionComp->IsActive()) return false;

	if (TriggerReactionType == EReactionType::None || TriggerReactionType == EReactionType::Max)
	{
		FLog::Log(TEXT("[AnimNotifyState_ReactionBase] Invalid TriggerReactionType."));
		return false;
	}

	const EReactionType reactionType = InReactionComp->GetActiveReactionType();

	if (TriggerReactionType != EReactionType::All && reactionType != TriggerReactionType) return false;

	return true;
}

UCReactionComponent* UCAnimNotifyState_ReactionBase::GetReactionComponent(USkeletalMeshComponent* InMeshComp) const
{
	if (!IsValid(InMeshComp)) return nullptr;

	ACharacter* ownerCharacter = Cast<ACharacter>(InMeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return nullptr;

	return ownerCharacter->FindComponentByClass<UCReactionComponent>();
}
