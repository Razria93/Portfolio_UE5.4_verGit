#include "Component/CMovementComponent.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

UCMovementComponent::UCMovementComponent()
{
}


void UCMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(GetOwner());
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


