#include "Character/Player/CPlayer.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CApplyDamageComponent.h"
#include "Component/CTakeDamageComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CReactionFeedbackComponent.h"

#include "Type/CWeaponStructure.h"
#include "Type/CStateStructure.h"

ACPlayer::ACPlayer()
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

	bUseControllerRotationYaw = false;

	// Init SpringArmComp
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	check(SpringArmComponent);
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
	SpringArmComponent->TargetArmLength = 300.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	// Init CameraComp
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	check(CameraComponent);
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 40.0f, 0.0f));
	CameraComponent->bUsePawnControlRotation = false;

	// Init MovementComp (Custom)
	MovementComponent = CreateDefaultSubobject<UCMovementComponent>(TEXT("Movement"));
	check(MovementComponent);

	// Init WeaponComp
	WeaponComponent = CreateDefaultSubobject<UCWeaponComponent>(TEXT("Weapon"));
	check(WeaponComponent);

	// Init StateComp
	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);

	// Init UCACtionComp
	ActionComponent = CreateDefaultSubobject<UCActionComponent>(TEXT("Action"));
	check(ActionComponent);

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

	// Init ReactionComp
	ReactionFeedbackComponent = CreateDefaultSubobject<UCReactionFeedbackComponent>(TEXT("ReactionFeedback"));
	check(ReactionFeedbackComponent);
}

void ACPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(HealthComponent) && IsValid(StateComponent))
	{
		HealthComponent->OnDeadStateChanged.AddUObject(StateComponent, &UCStateComponent::OnDeadStateChanged);
	}
}

void ACPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDeadStateChanged.RemoveAll(StateComponent);
	}

	Super::EndPlay(EndPlayReason);
}

void ACPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Consume and Execute pending reaction
	ConsumePendingReaction();
}

void ACPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ACPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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

void ACPlayer::HandleMoveForward(const float InAxisValue)
{
	if (!IsValid(Controller) || !IsValid(MovementComponent)) return;
	if (!HealthComponent || !HealthComponent->IsAlive()) return;

	MovementComponent->OnMoveForward(InAxisValue);
}

void ACPlayer::HandleMoveRight(const float InAxisValue)
{
	if (!IsValid(Controller) || !IsValid(MovementComponent)) return;
	if (!HealthComponent || !HealthComponent->IsAlive()) return;

	MovementComponent->OnMoveRight(InAxisValue);
}

void ACPlayer::HandleWalk()
{
	if (!IsValid(Controller) || !IsValid(MovementComponent)) return;
	if (!CanActionInput()) return;

	MovementComponent->OnWalk();
}

void ACPlayer::HandleRun()
{
	if (!IsValid(Controller) || !IsValid(MovementComponent)) return;
	if (!CanActionInput()) return;

	MovementComponent->OnRun();
}

void ACPlayer::HandleJump()
{
	if (!IsValid(Controller) || !IsValid(MovementComponent)) return;
	if (!CanActionInput()) return;

	Jump();
}

void ACPlayer::HandleStopJump()
{
	if (!IsValid(Controller) || !IsValid(MovementComponent)) return;

	StopJumping();
}

void ACPlayer::HandleComboAction()
{
	if (!IsValid(Controller) || !IsValid(ActionComponent)) return;
	if (!CanActionInput()) return;

	ActionComponent->SetComboAttackMode();
}

void ACPlayer::HandleSword()
{
	if (!IsValid(Controller) || !IsValid(WeaponComponent) || !IsValid(StateComponent)) return;
	if (!CanActionInput()) return;

	if (StateComponent->CheckCurStateType(EStateType::Idle))
	{
		if (WeaponComponent->CheckCurAttachmentType(EAttachmentType::Unarmed))
		{
			WeaponComponent->SetSwordMode();
		}
		else if (WeaponComponent->CheckCurAttachmentType(EAttachmentType::Sword))
		{
			WeaponComponent->SetUnarmedMode();
		}
	}
}

void ACPlayer::ConsumePendingReaction()
{
	if (!IsValid(HealthComponent)) return;
	if (!IsValid(ReactionComponent)) return;

	if (!HealthComponent->IsAlive()) return;

	if (!ReactionComponent->HasPendingReactionContext()) return;

	FReactionContext reactionContext;
	if (!ReactionComponent->TryConsumePendingReaction(reactionContext))
	{
		FLog::Log(TEXT("[Player|ConsumePendingReaction] Invalid Pending Reaction"));
		return;
	}

	if (!ReactionComponent->TryExecuteReaction(reactionContext))
	{
		FLog::Log(TEXT("[Player|ConsumePendingReaction] Rejected Execute Reaction"));
		return;
	}
}

bool ACPlayer::CanActionInput() const
{
	if (!IsValid(HealthComponent)) return false;
	if (!IsValid(StateComponent)) return true;

	if (!HealthComponent->IsAlive()) return false;

	const EStateType stateType = StateComponent->GetCurStateType();
	if (stateType == EStateType::Reaction) return false;
	if (stateType == EStateType::Dead) return false;

	return true;
}
