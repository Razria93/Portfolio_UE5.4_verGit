#include "Character/CAnimInstance.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CDefenseComponent.h"

void UCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter_Cached = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(OwnerCharacter_Cached)) return;

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Cached->GetComponentByClass(UCMovementComponent::StaticClass()));
	WeaponComp_Cached = Cast<UCWeaponComponent>(OwnerCharacter_Cached->GetComponentByClass(UCWeaponComponent::StaticClass()));
	HealthComp_Cached = Cast<UCHealthComponent>(OwnerCharacter_Cached->GetComponentByClass(UCHealthComponent::StaticClass()));
	DefenseComp_Cached = Cast<UCDefenseComponent>(OwnerCharacter_Cached->GetComponentByClass(UCDefenseComponent::StaticClass()));

	if (IsValid(WeaponComp_Cached))
	{
		WeaponComp_Cached->OnWeaponTypeChanged.AddUniqueDynamic(this, &UCAnimInstance::OnWeaponTypeChanged);
		CurrentWeaponType = WeaponComp_Cached->GetCurrentWeaponType();
	}
}

void UCAnimInstance::NativeUninitializeAnimation()
{
	if (IsValid(WeaponComp_Cached))
	{
		WeaponComp_Cached->OnWeaponTypeChanged.RemoveDynamic(this, &UCAnimInstance::OnWeaponTypeChanged);
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

	bIsGuarding = IsValid(DefenseComp_Cached) && DefenseComp_Cached->IsGuarding();
}

void UCAnimInstance::OnWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPrevWeaponType, EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(InOwnerCharacter) || (OwnerCharacter_Cached != InOwnerCharacter)) return;

	CurrentWeaponType = InNewWeaponType;
}
