#include "Type/CWeaponStructure.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "Components/ShapeComponent.h"

#include "Component/CMovementComponent.h"

bool FEquipmentData::IsValidMinimal() const
{
	return IsValid(Montage);
}

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
