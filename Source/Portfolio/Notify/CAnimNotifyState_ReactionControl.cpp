#include "Notify/CAnimNotifyState_ReactionControl.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"

#include "Type/CWeaponStructure.h"

UCAnimNotifyState_ReactionControl::UCAnimNotifyState_ReactionControl()
{
}

FString UCAnimNotifyState_ReactionControl::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Reaction");
}

FString UCAnimNotifyState_ReactionControl::MakeNotifyName(FString InName) const
{
	if (ReactionControlWindowType != EReactionControlWindowType::None)
	{
		UEnum* metaData = StaticEnum<EReactionControlWindowType>();

		if (metaData)
		{
			FString windowTypeName = metaData->GetNameStringByValue((int64)ReactionControlWindowType);
			return InName + "_" + windowTypeName;
		}
	}

	return InName;
}

void UCAnimNotifyState_ReactionControl::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
	if (!IsValid(ownerCharacter)) return;

	UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>();
	if (!IsValid(reactionComp)) return;

	EReactionNotifyCommand command = EReactionNotifyCommand::None;

	switch (ReactionControlWindowType)
	{
	case EReactionControlWindowType::Interruptible:
	{
		command = EReactionNotifyCommand::OpenInterruptWindow;
		break;
	}
	case EReactionControlWindowType::Cancelable:
	{
		command = EReactionNotifyCommand::OpenCancelWindow;
		break;
	}
	default:
		break;
	}

	reactionComp->HandleReactionNotifyCommand(command);
}

void UCAnimNotifyState_ReactionControl::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
	if (!IsValid(ownerCharacter)) return;

	UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>();
	if (!IsValid(reactionComp)) return;

	EReactionNotifyCommand command = EReactionNotifyCommand::None;

	switch (ReactionControlWindowType)
	{
	case EReactionControlWindowType::Interruptible:
	{
		command = EReactionNotifyCommand::CloseInterruptWindow;
		break;
	}
	case EReactionControlWindowType::Cancelable:
	{
		command = EReactionNotifyCommand::CloseCancelWindow;
		break;
	}
	default:
		break;
	}

	reactionComp->HandleReactionNotifyCommand(command);
}