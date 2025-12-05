#include "Character/CAnimInstance.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

#include "Component/CMovementComponent.h"

void UCAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(TryGetPawnOwner());
	check(OwnerCharacter_Cached);

	MovementComp_Cached = Cast<UCMovementComponent>(OwnerCharacter_Cached->GetComponentByClass(UCMovementComponent::StaticClass()));
	check(MovementComp_Cached);
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter_Cached) || !IsValid(MovementComp_Cached)) return;

	Speed = MovementComp_Cached->GetCurrentSpeed();
	Direction = MovementComp_Cached->GetCurrentDirection();
}