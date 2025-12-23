#include "Character/Enemy/CEnemy.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/CStateComponent.h"

ACEnemy::ACEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// Init CapsuleComp
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);

	// Init SkeletalMeshComp
	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // FRotator: (Pitch, Yaw, Roll)

	// Init CharacterMovementComp
	UCharacterMovementComponent* characterMovementComp = GetCharacterMovement();
	check(characterMovementComp);
	characterMovementComp->bOrientRotationToMovement = true;
	characterMovementComp->MaxWalkSpeed = 600.0f;

	// Init StateComp
	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);
}

void ACEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void ACEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

