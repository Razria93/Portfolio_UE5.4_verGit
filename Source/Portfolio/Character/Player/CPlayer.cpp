#include "Character/Player/CPlayer.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"

ACPlayer::ACPlayer()
{
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
}

void ACPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void ACPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACPlayer::HandleMoveForward(const float InAxisValue)
{
	if (IsValid(Controller) && IsValid(MovementComponent))
		MovementComponent->OnMoveForward(InAxisValue);
}

void ACPlayer::HandleMoveRight(const float InAxisValue)
{
	if (IsValid(Controller) && IsValid(MovementComponent))
		MovementComponent->OnMoveRight(InAxisValue);
}

void ACPlayer::HandleWalk()
{
	if (IsValid(Controller) && IsValid(MovementComponent))
		MovementComponent->OnWalk();
}

void ACPlayer::HandleRun()
{
	if (IsValid(Controller) && IsValid(MovementComponent))
		MovementComponent->OnRun();
}

void ACPlayer::HandleJump()
{
	if(IsValid(Controller) && IsValid(MovementComponent))
	Jump();
}
 
void ACPlayer::HandleStopJump()
{
	if (IsValid(Controller) && IsValid(MovementComponent))
	StopJumping();
}
