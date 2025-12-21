#include "Type/CWeaponStructure.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"

void FActionData::Begin_PlayMontage(ACharacter* InOwnerCharacter)
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

void FActionData::End_PlayMontage(ACharacter* InOwnerCharacter)
{
	UActorComponent* temp = InOwnerCharacter->GetComponentByClass(UCMovementComponent::StaticClass());
	if (!IsValid(temp)) return;

	UCMovementComponent* moveComp = Cast<UCMovementComponent>(temp);
	if (!IsValid(moveComp)) return;

	if (bCanMove == false)
		moveComp->SetMove();
}
