#include "Notify/CAnimNotify_SetIncapacitatedPresentation.h"

#include "Component/CBalanceComponent.h"
#include "Component/CReactionComponent.h"
#include "Core/Debug/FBalanceDebug.h"

#include "GameFramework/Character.h"

UCAnimNotify_SetIncapacitatedPresentation::UCAnimNotify_SetIncapacitatedPresentation()
{
}

FString UCAnimNotify_SetIncapacitatedPresentation::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Set Incapacitated Presentation: %s"), *UEnum::GetValueAsString(TargetPresentation));
}

void UCAnimNotify_SetIncapacitatedPresentation::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (TargetPresentation == EIncapacitatedPresentation::Max) return;

	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	ACharacter* ownerCharacter = IsValid(MeshComp) ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr;
	UCBalanceComponent* balanceComp = IsValid(ownerCharacter) ? ownerCharacter->FindComponentByClass<UCBalanceComponent>() : nullptr;
	if (IsValid(balanceComp))
	{
		FBalanceDebug::RecordLifecycleEvent(
			balanceComp,
			TEXT("IncapacitatedPresentationNotifyReceived"),
			FString::Printf(
				TEXT("Target=%s | Trigger=%s | Active=%s | ActiveReaction=%s"),
				*UEnum::GetValueAsString(TargetPresentation),
				*UEnum::GetValueAsString(TriggerReactionType),
				IsValid(reactionComp) && reactionComp->IsActive() ? TEXT("true") : TEXT("false"),
				IsValid(reactionComp) ? *UEnum::GetValueAsString(reactionComp->GetActiveReactionType()) : TEXT("Invalid")));
	}

	if (!CanProcessReactionNotify(reactionComp))
	{
		if (IsValid(balanceComp))
		{
			FBalanceDebug::RecordLifecycleEvent(balanceComp, TEXT("IncapacitatedPresentationNotifyFiltered"));
		}
		return;
	}

	if (IsValid(balanceComp))
	{
		FBalanceDebug::RecordLifecycleEvent(balanceComp, TEXT("IncapacitatedPresentationNotifyRouted"));
	}
	reactionComp->HandleReactionIncapacitatedPresentationNotify(TargetPresentation);
}
