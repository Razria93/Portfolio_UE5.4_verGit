#include "Notify/CAnimNotify_ReactionBase.h"

#include "ProjectGlobal.h"

#include "Component/CReactionComponent.h"
#include "Core/Debug/FAnimNotifyDebug.h"

#include "GameFramework/Character.h"

UCAnimNotify_ReactionBase::UCAnimNotify_ReactionBase()
{
}

bool UCAnimNotify_ReactionBase::CanProcessReactionNotify(const UCReactionComponent* InReactionComp) const
{
	if (!IsValid(InReactionComp)) return false;
	if (!InReactionComp->IsActive()) return false;

	if (TriggerReactionType == EReactionType::None || TriggerReactionType == EReactionType::Max)
	{
		FAnimNotifyDebug::ReportReactionNotifyTriggerWarning(this, InReactionComp->GetOwner(), InReactionComp, TriggerReactionType, TEXT("InvalidTriggerReactionType"));
		return false;
	}

	const EReactionType reactionType = InReactionComp->GetActiveReactionType();

	if (TriggerReactionType != EReactionType::All && reactionType != TriggerReactionType) return false;

	return true;
}

UCReactionComponent* UCAnimNotify_ReactionBase::GetReactionComponent(USkeletalMeshComponent* InMeshComp) const
{
	if (!IsValid(InMeshComp)) return nullptr;

	ACharacter* ownerCharacter = Cast<ACharacter>(InMeshComp->GetOwner());
	if (!IsValid(ownerCharacter)) return nullptr;

	return ownerCharacter->FindComponentByClass<UCReactionComponent>();
}
