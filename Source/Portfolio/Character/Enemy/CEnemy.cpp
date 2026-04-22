#include "Character/Enemy/CEnemy.h"
#include "ProjectGlobal.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CApplyDamageComponent.h"
#include "Component/CTakeDamageComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Component/CReactionFeedbackComponent.h"

#include "Type/CWeaponStructure.h"
#include "AI/Blackboard/CAIKey.h"

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

	// Init WeaponComp
	WeaponComponent = CreateDefaultSubobject<UCWeaponComponent>(TEXT("Weapon"));
	check(WeaponComponent);

	// Init StateComp
	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);

	// Init ApplyDamageComp
	ApplyDamageComponent = CreateDefaultSubobject<UCApplyDamageComponent>(TEXT("ApplyDamage"));
	check(ApplyDamageComponent);

	// Init TakeDamageComp
	TakeDamageComponent = CreateDefaultSubobject<UCTakeDamageComponent>(TEXT("TakeDamage"));
	check(TakeDamageComponent);

	// Init HealthComp
	HealthComponent = CreateDefaultSubobject<UCHealthComponent>(TEXT("Health"));
	check(HealthComponent);

	// Init ReactionComp
	ReactionComponent = CreateDefaultSubobject<UCReactionComponent>(TEXT("Reaction"));
	check(ReactionComponent);

	// Init ActionFeedbackComp
	ActionFeedbackComponent = CreateDefaultSubobject<UCActionFeedbackComponent>(TEXT("ActionFeedback"));
	check(ActionFeedbackComponent);

	// Init ReactionFeedbackComp
	ReactionFeedbackComponent = CreateDefaultSubobject<UCReactionFeedbackComponent>(TEXT("ReactionFeedback"));
	check(ReactionFeedbackComponent);
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

void ACEnemy::CacheActiveActionFeedbackKey(EActionType InActionType, int32 InActionIndex)
{
	ActiveActionFeedbackKey.ActionType = InActionType;
	ActiveActionFeedbackKey.ActionIndex = InActionIndex;
	bHasActiveActionFeedbackKey = true;
}

void ACEnemy::ClearActiveActionFeedbackKey()
{
	ActiveActionFeedbackKey.ActionType = EActionType::Max;
	ActiveActionFeedbackKey.ActionIndex = INDEX_NONE;
	bHasActiveActionFeedbackKey = false;
}

bool ACEnemy::TryStartKill()
{
	if (!IsValid(HealthComponent)) return false;

	return HealthComponent->TryKill();
}

bool ACEnemy::TryStartRevive(float InReviveHP)
{
	return IsValid(HealthComponent) && HealthComponent->TryRevive(InReviveHP);
}


