#include "Character/Enemy/CEnemy.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Component/CMovementComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CTakeDamageComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"

#include "Type/CWeaponStructure.h"

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

	// Init MovementComp (Custom)
	MovementComponent = CreateDefaultSubobject<UCMovementComponent>(TEXT("Movement"));
	check(MovementComponent);

	// Init StateComp
	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);

	// Init StateComp
	TakeDamageComponent = CreateDefaultSubobject<UCTakeDamageComponent>(TEXT("TakeDamage"));
	check(TakeDamageComponent);

	// Init HealthComp
	HealthComponent = CreateDefaultSubobject<UCHealthComponent>(TEXT("Health"));
	check(HealthComponent);

	// Init ReactionComp
	ReactionComponent = CreateDefaultSubobject<UCReactionComponent>(TEXT("Reaction"));
	check(ReactionComponent);
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

float ACEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Minimal validation
	if (DamageAmount <= 0.f) return 0.f;

	// TODO: Check DeadFlag and early return

	float finalDamage = DamageAmount;

	if (IsValid(TakeDamageComponent))
	{
		finalDamage = TakeDamageComponent->RequestTakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	else
	{
		// FallBack
		finalDamage = DamageAmount;
	}

	// Engine-Event Trigger
	Super::TakeDamage(finalDamage, DamageEvent, EventInstigator, DamageCauser);

	return finalDamage;
}


