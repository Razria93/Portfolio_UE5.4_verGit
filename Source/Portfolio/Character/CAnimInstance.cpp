#include "Character/CAnimInstance.h"

#include "ProjectGlobal.h"

#include "AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.h"
#include "Core/Profiling/CAIAnimationProfiling.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CDefenseComponent.h"

#include "GameFramework/Character.h"

// Lifecycle

void UCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	UnbindComponentEvents();
	ClearCachedReferences();

	InitializeAnimationStateForProfiling();

	if (!CacheOwnerAndComponents()) return;

	BindComponentEvents();

	RefreshMovementParameters();
	RefreshStateParameters();
}

void UCAnimInstance::NativeUninitializeAnimation()
{
	UnbindComponentEvents();
	ClearCachedReferences();

	ClearAnimationStateForProfiling();

	Super::NativeUninitializeAnimation();
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter_Cached)) return;
	if (!ShouldRefreshAnimationParameters(DeltaSeconds)) return;

	RefreshMovementParameters();
	RefreshStateParameters();
}

// Reference Cache

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

// Animation Profiling Gate

void UCAnimInstance::InitializeAnimationStateForProfiling()
{
	RuntimeLODAnimationRefreshElapsed = 0.f;
}

void UCAnimInstance::ClearAnimationStateForProfiling()
{
	RuntimeLODAnimationRefreshElapsed = 0.f;
}

bool UCAnimInstance::ShouldReduceEnemyAnimationRefreshForProfiling() const
{
	return FAIAnimationRuntimeLODPolicy::GetEnemyAnimationMode(OwnerCharacter_Cached) > 0;
}

bool UCAnimInstance::IsEnemyAnimationProfilingTarget() const
{
	return FAIAnimationRuntimeLODPolicy::IsEnemyAnimationRuntimeLODTarget(OwnerCharacter_Cached);
}

float UCAnimInstance::GetReducedAnimationRefreshIntervalForProfiling() const
{
	return FAIAnimationRuntimeLODPolicy::GetReducedAnimationRefreshInterval();
}

bool UCAnimInstance::ShouldRefreshAnimationParameters(float DeltaSeconds)
{
	RecordAnimationRefreshAttempt();

	if (!ShouldReduceEnemyAnimationRefreshForProfiling())
	{
		RuntimeLODAnimationRefreshElapsed = 0.f;
		RecordAnimationRefreshExecuted();
		return true;
	}

	RuntimeLODAnimationRefreshElapsed += DeltaSeconds;

	const float refreshInterval = GetReducedAnimationRefreshIntervalForProfiling();
	if (RuntimeLODAnimationRefreshElapsed < refreshInterval)
	{
		RecordAnimationRefreshSkipped();
		return false;
	}

	RuntimeLODAnimationRefreshElapsed = 0.f;
	RecordAnimationRefreshExecuted();
	return true;
}

// Animation Refresh Audit

bool UCAnimInstance::ShouldAuditAnimationRefreshForProfiling() const
{
	return IsEnemyAnimationProfilingTarget() && FAIAnimationProfiling::ShouldAuditAnimationRefresh();
}

void UCAnimInstance::RecordAnimationRefreshAttempt() const
{
	if (!ShouldAuditAnimationRefreshForProfiling()) return;

	FAIAnimationProfiling::RecordAnimationRefreshAttempt();
}

void UCAnimInstance::RecordAnimationRefreshExecuted() const
{
	if (!ShouldAuditAnimationRefreshForProfiling()) return;

	FAIAnimationProfiling::RecordAnimationRefreshExecuted();
}

void UCAnimInstance::RecordAnimationRefreshSkipped() const
{
	if (!ShouldAuditAnimationRefreshForProfiling()) return;

	FAIAnimationProfiling::RecordAnimationRefreshSkipped();
}

// Parameter Refresh

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

// Component Event Callback

void UCAnimInstance::OnWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPrevWeaponType, EWeaponType InNewWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(InOwnerCharacter) || (OwnerCharacter_Cached != InOwnerCharacter)) return;

	CurrentWeaponType = InNewWeaponType;
}
