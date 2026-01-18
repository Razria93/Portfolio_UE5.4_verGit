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
    if (ReactionWindowType != EReactionWindowType::None)
    {
        UEnum* metaData = StaticEnum<EReactionWindowType>();

        if (metaData)
        {
            FString windowTypeName = metaData->GetNameStringByValue((int64)ReactionWindowType);
            return InName + "_" + windowTypeName;
        }
    }

    return InName;
}

void UCAnimNotifyState_Reaction::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration);

    if (!IsValid(MeshComp)) return;

    AActor* ownerActor = MeshComp->GetOwner();
    ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
    if (!IsValid(ownerCharacter)) return;

    UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>();
    if (!IsValid(reactionComp)) return;

    reactionComp->OnReactionWindowBegin(ReactionWindowType, Animation);
}

void UCAnimNotifyState_Reaction::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::NotifyEnd(MeshComp, Animation);

    if (!IsValid(MeshComp)) return;

    AActor* ownerActor = MeshComp->GetOwner();
    ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
    if (!IsValid(ownerCharacter)) return;

    UCReactionComponent* reactionComp = ownerCharacter->FindComponentByClass<UCReactionComponent>();
    if (!IsValid(reactionComp)) return;

    reactionComp->OnReactionWindowEnd(ReactionWindowType, Animation);
}