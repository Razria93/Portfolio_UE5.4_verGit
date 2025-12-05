#include "Component/CMovementComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	if (!IsValid(OwnerCharacter_Cached)) return;
	if (!bCanMove) return;
	if (FMath::IsNearlyZero(InValue)) return;

	const FRotator controlRot = OwnerCharacter_Cached->GetControlRotation();
	const FRotator yawRot = FRotator(0.f, controlRot.Yaw, 0.f);
	const FVector Direction = FRotationMatrix(yawRot).GetUnitAxis(EAxis::X);

	OwnerCharacter_Cached->AddMovementInput(Direction, InValue);
}

void UCMovementComponent::OnMoveRight(float InValue)
{
	if (!IsValid(OwnerCharacter_Cached)) return;
	if (!bCanMove) return;
	if (FMath::IsNearlyZero(InValue)) return;

	const FRotator controlRot = OwnerCharacter_Cached->GetControlRotation();
	const FRotator yawRot = FRotator(0.f, controlRot.Yaw, 0.f);
	const FVector Direction = FRotationMatrix(yawRot).GetUnitAxis(EAxis::Y);

	OwnerCharacter_Cached->AddMovementInput(Direction, InValue);
}

void UCMovementComponent::OnWalk()
{
	SetSpeedType(ESpeedType::Walk);
}

void UCMovementComponent::OnRun()
{
	SetSpeedType(ESpeedType::Run);
}

void UCMovementComponent::OnSprint()
{
	SetSpeedType(ESpeedType::Sprint);
}

void UCMovementComponent::SetSpeedType(ESpeedType InType)
{
	if (!IsValid(OwnerCharacter_Cached) || !IsValid(CharacterMovementComp_Cached)) return;

	float newSpeed = SpeedMap[InType];
	CharacterMovementComp_Cached->MaxWalkSpeed = newSpeed;
}

void UCMovementComponent::CalculateSpeed()
{
	CurrentSpeed = OwnerCharacter_Cached->GetVelocity().Size2D();
}

void UCMovementComponent::CalculateDirection()
{
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
