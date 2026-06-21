#include "Component/CMovementComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/CStateComponent.h"

UCMovementComponent::UCMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
	check(OwnerCharacter_Cached);

	CharacterMovementComp_Cached = OwnerCharacter_Cached->GetCharacterMovement();
	check(CharacterMovementComp_Cached);

	StateComp_Cached = OwnerCharacter_Cached->FindComponentByClass<UCStateComponent>();
	check(StateComp_Cached);
}

void UCMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(CharacterMovementComp_Cached)) return;

	CalculateSpeed();
	CalculateDirection();

	bIsFalling = CharacterMovementComp_Cached->IsFalling();
}

// [Final Movement Gate]
// Final movement gate for axis input accepted by the orchestrator.
bool UCMovementComponent::CanAcceptMoveInput() const
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!bCanMove) return false;

	if (IsValid(StateComp_Cached))
	{
		const EExecutionState executionState = StateComp_Cached->GetCurrentExecutionState();

		if (executionState == EExecutionState::Dead) return false;
		if (executionState == EExecutionState::Reaction) return false;
	}

	return true;
}

void UCMovementComponent::OnMove(const FVector2D& InAxis2D)
{
	if (!CanAcceptMoveInput()) return;
	if (InAxis2D.IsNearlyZero()) return;
	if (!IsValid(OwnerCharacter_Cached)) return;

	const FRotator controlRot = OwnerCharacter_Cached->GetControlRotation();
	const FRotator yawRot = FRotator(0.f, controlRot.Yaw, 0.f);

	const FVector forwardDirection = FRotationMatrix(yawRot).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

	OwnerCharacter_Cached->AddMovementInput(forwardDirection, InAxis2D.Y);
	OwnerCharacter_Cached->AddMovementInput(rightDirection, InAxis2D.X);
}

void UCMovementComponent::OnWalk()
{
	ChangeMovementGait(EMovementGait::Walk);
}

void UCMovementComponent::OnRun()
{
	ChangeMovementGait(EMovementGait::Run);
}

void UCMovementComponent::OnSprint()
{
	ChangeMovementGait(EMovementGait::Sprint);
}

void UCMovementComponent::OnJump()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	OwnerCharacter_Cached->Jump();
}

void UCMovementComponent::OnStopJump()
{
	if (!IsValid(OwnerCharacter_Cached)) return;

	OwnerCharacter_Cached->StopJumping();
}

void UCMovementComponent::ApplyMovementOverride(EMovementGait InGait, EMovementRotationMode InRotationMode)
{
	if (!bHasMovementModeOverride)
	{
		CachedMovementGait_BeforeOverride = CurrentMovementGait;
		bHasMovementModeOverride = true;
	}

	ApplyRotationMode(InRotationMode);
	ApplyMovementGait(InGait);
}

void UCMovementComponent::ClearMovementOverride()
{
	ApplyRotationMode(EMovementRotationMode::OrientToMovement);

	if (!bHasMovementModeOverride) return;

	bHasMovementModeOverride = false;
	ApplyMovementGait(CachedMovementGait_BeforeOverride);
}

void UCMovementComponent::ChangeMovementGait(EMovementGait InNewMovementGait)
{
	if (bHasMovementModeOverride)
	{
		// Keep override speed active, but remember the base gait to restore later.
		CachedMovementGait_BeforeOverride = InNewMovementGait;
		return;
	}

	ApplyMovementGait(InNewMovementGait);
}

void UCMovementComponent::ApplyMovementGait(EMovementGait InNewMovementGait)
{
	if (!IsValid(CharacterMovementComp_Cached)) return;
	if (InNewMovementGait == EMovementGait::None || InNewMovementGait == EMovementGait::Max) return;

	const float* speed = GaitSpeedMap.Find(InNewMovementGait);

	if (!speed)
	{
		FLog::Log(TEXT("[ChangeMovementGait] InValid GaitSpeedMap")); // Error
		return;
	}

	CurrentMovementGait = InNewMovementGait;
	CharacterMovementComp_Cached->MaxWalkSpeed = *speed;
}

void UCMovementComponent::ApplyRotationMode(EMovementRotationMode InRotationMode)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(CharacterMovementComp_Cached)) return;

	switch (InRotationMode)
	{
	case EMovementRotationMode::OrientToMovement:
		CharacterMovementComp_Cached->bOrientRotationToMovement = true;
		CharacterMovementComp_Cached->bUseControllerDesiredRotation = false;
		OwnerCharacter_Cached->bUseControllerRotationYaw = false;
		break;

	case EMovementRotationMode::ControllerDesired:
		CharacterMovementComp_Cached->bOrientRotationToMovement = false;
		CharacterMovementComp_Cached->bUseControllerDesiredRotation = true;
		OwnerCharacter_Cached->bUseControllerRotationYaw = false;
		break;

	case EMovementRotationMode::FixedFacing:
		CharacterMovementComp_Cached->bOrientRotationToMovement = false;
		CharacterMovementComp_Cached->bUseControllerDesiredRotation = false;
		OwnerCharacter_Cached->bUseControllerRotationYaw = false;
		break;

	default:
		break;
	}
}

void UCMovementComponent::CalculateSpeed()
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(CharacterMovementComp_Cached)) return;

	CurrentSpeed = OwnerCharacter_Cached->GetVelocity().Size2D();
}

void UCMovementComponent::CalculateDirection()
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(CharacterMovementComp_Cached)) return;

	if (CurrentSpeed < KINDA_SMALL_NUMBER)
	{
		CurrentDirection = 0.f;
		return;
	}
	else
	{
		const FVector velocityNormal = OwnerCharacter_Cached->GetVelocity().GetSafeNormal2D();
		const FVector forwardNormal = OwnerCharacter_Cached->GetActorForwardVector().GetSafeNormal2D();

		float angleRad = FMath::Acos(FVector::DotProduct(forwardNormal, velocityNormal));
		float angleDeg = FMath::RadiansToDegrees(angleRad);

		// determine left or right
		FVector crossProduct = FVector::CrossProduct(forwardNormal, velocityNormal);
		if (crossProduct.Z < 0)
		{
			angleDeg *= -1.f;
		}

		CurrentDirection = angleDeg;
	}
}
