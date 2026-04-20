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

void UCMovementComponent::OnMoveForward(float InValue)
{
	if (!CanAcceptMoveInput()) return;
	if (FMath::IsNearlyZero(InValue)) return;

	const FRotator controlRot = OwnerCharacter_Cached->GetControlRotation();
	const FRotator yawRot = FRotator(0.f, controlRot.Yaw, 0.f);
	const FVector Direction = FRotationMatrix(yawRot).GetUnitAxis(EAxis::X);

	OwnerCharacter_Cached->AddMovementInput(Direction, InValue);
}

void UCMovementComponent::OnMoveRight(float InValue)
{
	if (!CanAcceptMoveInput()) return;
	if (FMath::IsNearlyZero(InValue)) return;

	const FRotator controlRot = OwnerCharacter_Cached->GetControlRotation();
	const FRotator yawRot = FRotator(0.f, controlRot.Yaw, 0.f);
	const FVector Direction = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

	OwnerCharacter_Cached->AddMovementInput(Direction, InValue);
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

// [Final Movement Gate]
// Axis move input bypasses the orchestrator
bool UCMovementComponent::CanAcceptMoveInput() const
{
	if (!IsValid(OwnerCharacter_Cached)) return false;
	if (!bCanMove) return false;

	if (IsValid(StateComp_Cached))
	{
		const EExecutionState executionState = StateComp_Cached->GetCurExecutionState();

		if (executionState == EExecutionState::Dead) return false;
		if (executionState == EExecutionState::Reaction) return false;
	}

	return true;
}

void UCMovementComponent::ChangeMovementGait(EMovementGait InNewMovementGait)
{
	if (!IsValid(CharacterMovementComp_Cached)) return;
	if (InNewMovementGait == EMovementGait::None || InNewMovementGait == EMovementGait::Max) return;

	const float* speed = GaitSpeedMap.Find(InNewMovementGait);
	if (!speed) return;
	if (!speed)
	{
		FLog::Log(TEXT("[ChangeMovementGait] InValid GaitSpeedMap")); // Error
		return;
	}

	CurrentMovementGait = InNewMovementGait;
	CharacterMovementComp_Cached->MaxWalkSpeed = *speed;
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
