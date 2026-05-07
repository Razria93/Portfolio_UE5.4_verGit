#include "Notify/CAnimNotifyState_Reaction.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CReactionComponent.h"

#include "Type/CWeaponStructure.h"

UCAnimNotifyState_Reaction::UCAnimNotifyState_Reaction()
{
}

FString UCAnimNotifyState_Reaction::GetNotifyName_Implementation() const
{
	return MakeNotifyName("Reaction");
}

FString UCAnimNotifyState_Reaction::MakeNotifyName(FString InName) const
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

void UCAnimNotifyState_Reaction::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
	if (!IsValid(ownerCharacter)) return;

	UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>();
	if (!IsValid(reactionComp)) return;

	reactionComp->HandleReactionControlWindowBegin(ReactionControlWindowType);
}

void UCAnimNotifyState_Reaction::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
	if (!IsValid(ownerCharacter)) return;

	UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>();
	if (!IsValid(reactionComp)) return;

	reactionComp->HandleReactionControlWindowEnd(ReactionControlWindowType);
}