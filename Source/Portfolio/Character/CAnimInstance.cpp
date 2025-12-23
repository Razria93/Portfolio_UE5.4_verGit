#include "Character/CAnimInstance.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"

void UCAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(OwnerCharacter_Cached)) return;

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Cached->GetComponentByClass(UCMovementComponent::StaticClass()));
	if (!IsValid(MovementComp_Cached)) return;

	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerCharacter_Cached->GetComponentByClass(UCWeaponComponent::StaticClass()));
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->OnWeaponTypeChanged.AddDynamic(this, &UCAnimInstance::OnWeaponTypeChanged);
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached)) return;

	Speed = MovementComp_Cached->GetCurrentSpeed();
	Direction = MovementComp_Cached->GetCurrentDirection();
	bIsInAir = MovementComp_Cached->IsFalling();
}

void UCAnimInstance::OnWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPrevWeaponType, EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(InOwnerCharacter) || (OwnerCharacter_Cached != InOwnerCharacter)) return;

	WeaponType = InNewWeaponType;
}
