#include "Character/CAnimInstance.h"

#include "ProjectGlobal.h"

#include "AI/RuntimeLOD/CAIAnimationRuntimeLODPolicy.h"
#include "Core/Profiling/CAIAnimationProfiling.h"
#include "Core/Debug/FBalanceDebug.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CStateComponent.h"
#include "Character/Enemy/CEnemy.h"

#include "GameFramework/Character.h"

namespace
{
	ELocomotionPresentationMode ResolveLocomotionPresentationModeFromRotationMode(EMovementRotationMode InRotationMode)
	{
		switch (InRotationMode)
		{
		case EMovementRotationMode::ControllerDesired:
		case EMovementRotationMode::FixedFacing:
			return ELocomotionPresentationMode::Directional;

		case EMovementRotationMode::OrientToMovement:
		case EMovementRotationMode::None:
		case EMovementRotationMode::Max:
		default:
			return ELocomotionPresentationMode::Forward;
		}
	}
}

// Lifecycle

void UCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	UnbindComponentEvents();
	ResetAnimationParameters();
	ClearCachedComponentReferences();
	ResetAnimationRefreshThrottle();

	if (!CacheOwnerAndComponentReferences()) return;

	BindComponentEvents();

	RefreshMovementParameters();
	RefreshStateParameters();
}

void UCAnimInstance::NativeUninitializeAnimation()
{
	UnbindComponentEvents();
	ResetAnimationParameters();
	ClearCachedComponentReferences();
	ResetAnimationRefreshThrottle();

	Super::NativeUninitializeAnimation();
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter_Cached)) return;
	if (!TryConsumeAnimationRefreshGate(DeltaSeconds)) return;

	RefreshMovementParameters();
	RefreshStateParameters();
}

// Reference Lifecycle

bool UCAnimInstance::CacheOwnerAndComponentReferences()
{
	OwnerCharacter_Cached = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(OwnerCharacter_Cached)) return false;

	MovementComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCMovementComponent>();
	WeaponComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCWeaponComponent>();
	HealthComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCHealthComponent>();
	DefenseComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCDefenseComponent>();
	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	BalanceComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCBalanceComponent>();

	return true;
}

void UCAnimInstance::ClearCachedComponentReferences()
{
	OwnerCharacter_Cached = nullptr;
	MovementComp_Cached = nullptr;
	WeaponComp_Cached = nullptr;
	HealthComp_Cached = nullptr;
	DefenseComp_Cached = nullptr;
	StateComp_Cached = nullptr;
	BalanceComp_Cached = nullptr;
}

void UCAnimInstance::BindComponentEvents()
{
	if (IsValid(WeaponComp_Cached))
	{
		WeaponComp_Cached->OnWeaponTypeChanged.AddUniqueDynamic(this, &UCAnimInstance::HandleWeaponTypeChanged);
		CurrentWeaponType = WeaponComp_Cached->GetCurrentWeaponType();
	}

	if (IsValid(HealthComp_Cached))
	{
		HealthComp_Cached->OnDeadStateChanged.AddUObject(this, &UCAnimInstance::HandleDeadStateChanged);
		HandleDeadStateChanged(EDeadState::Alive, HealthComp_Cached->GetDeadState());
	}

	if (IsValid(BalanceComp_Cached))
	{
		BalanceComp_Cached->OnBalanceLifecycleStateChanged.AddUObject(this, &UCAnimInstance::HandleBalanceLifecycleStateChanged);
		BalanceComp_Cached->OnIncapacitatedPresentationChanged.AddUObject(this, &UCAnimInstance::HandleIncapacitatedPresentationChanged);
		ApplyIncapacitatedPresentationState(BalanceComp_Cached->GetIncapacitatedPresentation());
		bIsCollapsePose = BalanceComp_Cached->IsCollapseActive();
	}
}

void UCAnimInstance::UnbindComponentEvents()
{
	if (IsValid(WeaponComp_Cached))
	{
		WeaponComp_Cached->OnWeaponTypeChanged.RemoveDynamic(this, &UCAnimInstance::HandleWeaponTypeChanged);
	}

	if (IsValid(HealthComp_Cached))
	{
		HealthComp_Cached->OnDeadStateChanged.RemoveAll(this);
	}

	if (IsValid(BalanceComp_Cached))
	{
		BalanceComp_Cached->OnBalanceLifecycleStateChanged.RemoveAll(this);
		BalanceComp_Cached->OnIncapacitatedPresentationChanged.RemoveAll(this);
	}
}

// Animation Parameter Lifecycle

void UCAnimInstance::RefreshMovementParameters()
{
	if (IsValid(MovementComp_Cached))
	{
		Speed = MovementComp_Cached->GetCurrentSpeed();
		Direction = MovementComp_Cached->GetCurrentDirection();
		bIsInAir = MovementComp_Cached->IsFalling();
		LocomotionPresentationMode = ResolveLocomotionPresentationModeFromRotationMode(MovementComp_Cached->GetCurrentMovementRotationMode());
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
		bIsDeadPose = HealthComp_Cached->IsDead();
	}

	bIsGuardingPose = IsValid(DefenseComp_Cached) && DefenseComp_Cached->IsGuardingPose();
	ApplyIncapacitatedPresentationState(IsValid(BalanceComp_Cached)
		? BalanceComp_Cached->GetIncapacitatedPresentation()
		: EIncapacitatedPresentation::None);
	// Compatibility: the current AnimGraph still reads the two bools. Its
	// migration to IncapacitatedPresentation is intentionally asset-side.
	bIsCollapsePose = IsValid(BalanceComp_Cached) && BalanceComp_Cached->IsCollapseActive();
	DeathPresentationMode = EDeathPresentationMode::Default;
	if (const ACEnemy* enemy = Cast<ACEnemy>(OwnerCharacter_Cached))
	{
		DeathPresentationMode = enemy->GetDeathPresentationMode();
	}
	CurrentExecutionState = IsValid(StateComp_Cached) ? StateComp_Cached->GetCurrentExecutionState() : EExecutionState::Max;
}

void UCAnimInstance::ResetAnimationParameters()
{
	Speed = 0.f;
	Direction = 0.f;
	bIsInAir = false;
	LocomotionPresentationMode = ELocomotionPresentationMode::Forward;

	CurrentWeaponType = EWeaponType::Max;
	bIsDeadPose = false;
	ApplyIncapacitatedPresentationState(EIncapacitatedPresentation::None);
	bIsCollapsePose = false;
	DeathPresentationMode = EDeathPresentationMode::Default;
	CurrentExecutionState = EExecutionState::Max;
	bIsGuardingPose = false;
}

void UCAnimInstance::ApplyIncapacitatedPresentationState(const EIncapacitatedPresentation InPresentation)
{
	IncapacitatedPresentation = InPresentation;
	bIsIncapacitatedPose = IncapacitatedPresentation != EIncapacitatedPresentation::None;

	// Compatibility input until all AnimGraph references migrate to the enum.
	bIsExecutionDownPose = IncapacitatedPresentation == EIncapacitatedPresentation::ExecutionDown;
}

// Runtime LOD Animation Refresh Gate

bool UCAnimInstance::TryConsumeAnimationRefreshGate(float DeltaSeconds)
{
	RecordAnimationRefreshAttempt();

	const int32 animationMode = FAIAnimationRuntimeLODPolicy::GetEnemyAnimationMode(OwnerCharacter_Cached);
	if (animationMode <= 0)
	{
		AnimationRefreshThrottleElapsedSeconds = 0.f;
		RecordAnimationRefreshExecuted();
		return true;
	}

	AnimationRefreshThrottleElapsedSeconds += DeltaSeconds;

	const float refreshInterval = FAIAnimationRuntimeLODPolicy::GetReducedAnimationRefreshInterval();
	if (AnimationRefreshThrottleElapsedSeconds < refreshInterval)
	{
		RecordAnimationRefreshSkipped();
		return false;
	}

	AnimationRefreshThrottleElapsedSeconds = 0.f;
	RecordAnimationRefreshExecuted();
	return true;
}

void UCAnimInstance::ResetAnimationRefreshThrottle()
{
	AnimationRefreshThrottleElapsedSeconds = 0.f;
}

// Animation Refresh Audit

bool UCAnimInstance::ShouldRecordAnimationRefreshAudit() const
{
	return FAIAnimationRuntimeLODPolicy::IsEnemyAnimationRuntimeLODTarget(OwnerCharacter_Cached)
		&& FAIAnimationProfiling::ShouldAuditAnimationRefresh();
}

void UCAnimInstance::RecordAnimationRefreshAttempt() const
{
	if (!ShouldRecordAnimationRefreshAudit()) return;

	FAIAnimationProfiling::RecordAnimationRefreshAttempt();
}

void UCAnimInstance::RecordAnimationRefreshExecuted() const
{
	if (!ShouldRecordAnimationRefreshAudit()) return;

	FAIAnimationProfiling::RecordAnimationRefreshExecuted();
}

void UCAnimInstance::RecordAnimationRefreshSkipped() const
{
	if (!ShouldRecordAnimationRefreshAudit()) return;

	FAIAnimationProfiling::RecordAnimationRefreshSkipped();
}

// Component Event Handlers

void UCAnimInstance::HandleWeaponTypeChanged(ACharacter* InOwnerCharacter, EWeaponType InPreviousWeaponType, EWeaponType InCurrentWeaponType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(InOwnerCharacter) || (OwnerCharacter_Cached != InOwnerCharacter)) return;

	CurrentWeaponType = InCurrentWeaponType;
}

void UCAnimInstance::HandleDeadStateChanged(EDeadState InPreviousDeadState, EDeadState InCurrentDeadState)
{
	bIsDeadPose = InCurrentDeadState == EDeadState::Dead;
}

void UCAnimInstance::HandleBalanceLifecycleStateChanged(const EBalanceLifecycleState InPreviousState, const EBalanceLifecycleState InCurrentState)
{
	bIsCollapsePose = IsValid(BalanceComp_Cached) && BalanceComp_Cached->IsCollapseActive();
}

void UCAnimInstance::HandleIncapacitatedPresentationChanged(const EIncapacitatedPresentation InPresentation)
{
	ApplyIncapacitatedPresentationState(InPresentation);
	if (IsValid(BalanceComp_Cached))
	{
		FBalanceDebug::RecordLifecycleEvent(
			BalanceComp_Cached,
			TEXT("IncapacitatedPresentationAnimUpdated"),
			FString::Printf(
				TEXT("Presentation=%s | IsIncapacitatedPose=%s | LegacyExecutionDownPose=%s"),
				*UEnum::GetValueAsString(IncapacitatedPresentation),
				bIsIncapacitatedPose ? TEXT("true") : TEXT("false"),
				bIsExecutionDownPose ? TEXT("true") : TEXT("false")));
	}
}
