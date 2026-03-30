#include "Character/CAnimInstance.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CHealthComponent.h"

void UCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter_Cached = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(OwnerCharacter_Cached)) return;

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Cached->GetComponentByClass(UCMovementComponent::StaticClass()));
	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerCharacter_Cached->GetComponentByClass(UCWeaponComponent::StaticClass()));
	HealthComp_Cached = Cast<UCHealthComponent>(OwnerCharacter_Cached->GetComponentByClass(UCHealthComponent::StaticClass()));

	if (IsValid(WeaponComp_Cached))
	{
		WeaponComp_Cached->OnAttachmentTypeChanged.AddUniqueDynamic(this, &UCAnimInstance::OnAttachmentTypeChanged);
		AttachmentType = WeaponComp_Cached->GetCurAttachmentType();
	}
}

void UCAnimInstance::NativeUninitializeAnimation()
{
	if (IsValid(WeaponComp_Cached))
	{
		WeaponComp_Cached->OnAttachmentTypeChanged.RemoveDynamic(this, &UCAnimInstance::OnAttachmentTypeChanged);
	}

	Super::NativeUninitializeAnimation();
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter_Cached)) return;

	if (IsValid(MovementComp_Cached))
	{
		Speed = MovementComp_Cached->GetCurrentSpeed();
		Direction = MovementComp_Cached->GetCurrentDirection();
		bIsInAir = MovementComp_Cached->IsFalling();
	}

	if (IsValid(HealthComp_Cached))
	{
		DeadState = HealthComp_Cached->GetDeadState();
	}
}

void UCAnimInstance::OnAttachmentTypeChanged(ACharacter* InOwnerCharacter, EAttachmentType InPrevAttachmentType, EAttachmentType InNewAttachmentType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(InOwnerCharacter) || (OwnerCharacter_Cached != InOwnerCharacter)) return;

	AttachmentType = InNewAttachmentType;
}
