#include "Notify/CAnimNotify_EnterExecutionDownPresentation.h"

#include "Component/CBalanceComponent.h"
#include "Component/CReactionComponent.h"
#include "Core/Debug/FBalanceDebug.h"

#include "GameFramework/Character.h"

UCAnimNotify_EnterExecutionDownPresentation::UCAnimNotify_EnterExecutionDownPresentation()
{
	TriggerReactionType = EReactionType::ExecutionStandard;
}

FString UCAnimNotify_EnterExecutionDownPresentation::GetNotifyName_Implementation() const
{
	return TEXT("Enter Execution Down Presentation");
}

void UCAnimNotify_EnterExecutionDownPresentation::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	UCReactionComponent* reactionComp = GetReactionComponent(MeshComp);
	ACharacter* ownerCharacter = IsValid(MeshComp) ? Cast<ACharacter>(MeshComp->GetOwner()) : nullptr;
	UCBalanceComponent* balanceComp = IsValid(ownerCharacter) ? ownerCharacter->FindComponentByClass<UCBalanceComponent>() : nullptr;

	if (IsValid(balanceComp))
	{
		FBalanceDebug::RecordLifecycleEvent(
			balanceComp,
			TEXT("ExecutionDownPresentationNotifyReceived"),
			FString::Printf(
				TEXT("Trigger=ExecutionStandard | ReactionComp=%s | Active=%s | ActiveReaction=%s"),
				*GetNameSafe(reactionComp),
				IsValid(reactionComp) && reactionComp->IsActive() ? TEXT("true") : TEXT("false"),
				IsValid(reactionComp) ? *UEnum::GetValueAsString(reactionComp->GetActiveReactionType()) : TEXT("Invalid")));
	}

	if (!CanProcessReactionNotify(reactionComp))
	{
		if (IsValid(balanceComp))
		{
			FBalanceDebug::RecordLifecycleEvent(balanceComp, TEXT("ExecutionDownPresentationNotifyFiltered"));
		}
		return;
	}

	if (IsValid(balanceComp))
	{
		FBalanceDebug::RecordLifecycleEvent(balanceComp, TEXT("ExecutionDownPresentationNotifyRouted"));
	}
	reactionComp->HandleReactionNotifyCommand(EReactionNotifyCommand::EnterExecutionDownPresentation);
}
