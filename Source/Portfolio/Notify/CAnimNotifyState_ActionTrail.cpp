#include "Notify/CAnimNotifyState_ActionTrail.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CActionComponent.h"
#include "Action/CAction.h"

UCAnimNotifyState_ActionTrail::UCAnimNotifyState_ActionTrail()
{
}

FString UCAnimNotifyState_ActionTrail::GetNotifyName_Implementation() const
{
	return TEXT("ActionTrail");
}

void UCAnimNotifyState_ActionTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!IsValid(actionComp)) return;

	UObject* uobject = actionComp->GetCurAction();
	if (!uobject) return;

	UCAction* curAction = Cast<UCAction>(uobject);
	if (!curAction) return;

	FLog::Log(TEXT("[AnimNotifyState|Trail] Begin"));
	curAction->NotifyActionTrailBegin();
}

void UCAnimNotifyState_ActionTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp)) return;

	AActor* ownerActor = MeshComp->GetOwner();
	ACharacter* ownerCharacter = Cast<ACharacter>(ownerActor);
	if (!IsValid(ownerCharacter)) return;

	UCActionComponent* actionComp = ownerCharacter->FindComponentByClass<UCActionComponent>();
	if (!IsValid(actionComp)) return;

	UObject* uobject = actionComp->GetCurAction();
	if (!uobject) return;

	UCAction* curAction = Cast<UCAction>(uobject);
	if (!curAction) return;

	FLog::Log(TEXT("[AnimNotifyState|Trail] End"));
	curAction->NotifyActionTrailEnd();
}

