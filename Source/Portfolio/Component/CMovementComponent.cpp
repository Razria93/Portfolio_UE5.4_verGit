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

void UCMovementComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	OwnerCharacter_Injected = InReferences.OwnerCharacter;
	CharacterMovementComp_Injected = IsValid(OwnerCharacter_Injected) ? OwnerCharacter_Injected->GetCharacterMovement() : nullptr;
	StateComp_Injected = InReferences.StateComponent;

	ValidateRequiredComponentReferences();
}

bool UCMovementComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ OwnerCharacter_Injected, TEXT("ACharacter Owner") },
		{ CharacterMovementComp_Injected, TEXT("UCharacterMovementComponent") },
		{ StateComp_Injected, TEXT("UCStateComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, OwnerCharacter_Injected, this);
	}

	return bValid;
}

void UCMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	CalculateSpeed();
	CalculateDirection();

	bIsFalling = CharacterMovementComp_Injected->IsFalling();
}

// [Final Movement Gate]
// Final movement gate for axis input accepted by the orchestrator.
bool UCMovementComponent::CanAcceptMoveInput() const
{
	if (!IsValid(OwnerCharacter_Injected)) return false;
	if (!bCanMove) return false;

	if (IsValid(StateComp_Injected))
	{
		const EExecutionState executionState = StateComp_Injected->GetCurrentExecutionState();

		if (executionState == EExecutionState::Dead) return false;
		if (executionState == EExecutionState::Reaction) return false;
	}

	return true;
}

void UCMovementComponent::OnMove(const FVector2D& InAxis2D)
{
	if (!CanAcceptMoveInput()) return;
	if (InAxis2D.IsNearlyZero()) return;
	if (!IsValid(OwnerCharacter_Injected)) return;

	const FRotator controlRot = OwnerCharacter_Injected->GetControlRotation();
	const FRotator yawRot = FRotator(0.f, controlRot.Yaw, 0.f);

	const FVector forwardDirection = FRotationMatrix(yawRot).GetUnitAxis(EAxis::X);
	const FVector rightDirection = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

	OwnerCharacter_Injected->AddMovementInput(forwardDirection, InAxis2D.Y);
	OwnerCharacter_Injected->AddMovementInput(rightDirection, InAxis2D.X);
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
	if (!IsValid(OwnerCharacter_Injected)) return;

	OwnerCharacter_Injected->Jump();
}

void UCMovementComponent::OnStopJump()
{
	if (!IsValid(OwnerCharacter_Injected)) return;

	OwnerCharacter_Injected->StopJumping();
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
	if (!IsValid(CharacterMovementComp_Injected)) return;
	if (InNewMovementGait == EMovementGait::None || InNewMovementGait == EMovementGait::Max) return;

	const float* speed = GaitSpeedMap.Find(InNewMovementGait);

	if (!speed)
	{
		FLog::Log(TEXT("[ChangeMovementGait] InValid GaitSpeedMap")); // Error
		return;
	}

	CurrentMovementGait = InNewMovementGait;
	CharacterMovementComp_Injected->MaxWalkSpeed = *speed;
}

void UCMovementComponent::ApplyRotationMode(EMovementRotationMode InRotationMode)
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	switch (InRotationMode)
	{
	case EMovementRotationMode::OrientToMovement:
		CharacterMovementComp_Injected->bOrientRotationToMovement = true;
		CharacterMovementComp_Injected->bUseControllerDesiredRotation = false;
		OwnerCharacter_Injected->bUseControllerRotationYaw = false;
		break;

	case EMovementRotationMode::ControllerDesired:
		CharacterMovementComp_Injected->bOrientRotationToMovement = false;
		CharacterMovementComp_Injected->bUseControllerDesiredRotation = true;
		OwnerCharacter_Injected->bUseControllerRotationYaw = false;
		break;

	case EMovementRotationMode::FixedFacing:
		CharacterMovementComp_Injected->bOrientRotationToMovement = false;
		CharacterMovementComp_Injected->bUseControllerDesiredRotation = false;
		OwnerCharacter_Injected->bUseControllerRotationYaw = false;
		break;

	default:
		break;
	}
}

void UCMovementComponent::CalculateSpeed()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	CurrentSpeed = OwnerCharacter_Injected->GetVelocity().Size2D();
}

void UCMovementComponent::CalculateDirection()
{
	if (!IsValid(OwnerCharacter_Injected) || !IsValid(CharacterMovementComp_Injected)) return;

	if (CurrentSpeed < KINDA_SMALL_NUMBER)
	{
		CurrentDirection = 0.f;
		return;
	}
	else
	{
		const FVector velocityNormal = OwnerCharacter_Injected->GetVelocity().GetSafeNormal2D();
		const FVector forwardNormal = OwnerCharacter_Injected->GetActorForwardVector().GetSafeNormal2D();

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
