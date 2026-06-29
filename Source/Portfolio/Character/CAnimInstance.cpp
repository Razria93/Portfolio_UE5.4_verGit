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

	UnbindComponentEvents();
	ClearCachedReferences();

	if (!CacheOwnerAndComponents()) return;

	BindComponentEvents();
	RefreshStateParameters();
}

void UCAnimInstance::NativeUninitializeAnimation()
{
	UnbindComponentEvents();
	ClearCachedReferences();

	Super::NativeUninitializeAnimation();
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter_Cached)) return;

	RefreshMovementParameters();
	RefreshStateParameters();
}

bool UCAnimInstance::CacheOwnerAndComponents()
{
	OwnerCharacter_Cached = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(OwnerCharacter_Cached)) return false;

	MovementComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCMovementComponent>();
	WeaponComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCWeaponComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	DefenseComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCDefenseComponent>();

	return true;
}

void UCAnimInstance::ClearCachedReferences()
{
	OwnerCharacter_Cached = nullptr;
	MovementComp_Cached = nullptr;
	WeaponComp_Cached = nullptr;
	HealthComp_Cached = nullptr;
	DefenseComp_Cached = nullptr;
}

void UCAnimInstance::BindComponentEvents()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->OnWeaponTypeChanged.AddUniqueDynamic(this, &UCAnimInstance::OnWeaponTypeChanged);
	CurrentWeaponType = WeaponComp_Cached->GetCurrentWeaponType();
}

void UCAnimInstance::UnbindComponentEvents()
{
	if (!IsValid(WeaponComp_Cached)) return;

	WeaponComp_Cached->OnWeaponTypeChanged.RemoveDynamic(this, &UCAnimInstance::OnWeaponTypeChanged);
}

void UCAnimInstance::RefreshMovementParameters()
{
	if (IsValid(MovementComp_Cached))
	{
		Speed = MovementComp_Cached->GetCurrentSpeed();
		Direction = MovementComp_Cached->GetCurrentDirection();
		bIsInAir = MovementComp_Cached->IsFalling();
	}
}

void UCAnimInstance::RefreshStateParameters()
{
	if (IsValid(WeaponComp_Cached))
	{
		CurrentWeaponType = WeaponComp_Cached->GetCurrentWeaponType();
	}

	if (IsValid(HealthComp_Cached))
	{
		DeadState = HealthComp_Cached->GetDeadState();
	}

	bIsGuardingPose = IsValid(DefenseComp_Cached) && DefenseComp_Cached->IsGuardingPose();
}

void UCAnimInstance::OnWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPrevWeaponType, EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(InOwnerCharacter) || (OwnerCharacter_Cached != InOwnerCharacter)) return;

	CurrentWeaponType = InNewWeaponType;
}
