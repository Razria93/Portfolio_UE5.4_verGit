#include "Character/CAnimInstance.h"
#include "ProjectGlobal.h"

#include "GameFramework/Character.h"

void UCAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OwnerCharacter_Cached = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(OwnerCharacter_Cached)) return;
}

void UCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(OwnerCharacter_Cached)) return;

	// Calculate Speed
	Speed = OwnerCharacter_Cached->GetVelocity().Size2D();

	if (Speed < KINDA_SMALL_NUMBER)
	{
		Direction = 0.f;
		return;
	}

	// Calculate Direction
	Direction = CalculateDirection(OwnerCharacter_Cached->GetVelocity(), OwnerCharacter_Cached->GetActorRotation());
}