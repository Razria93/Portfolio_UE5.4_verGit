#include "Character/Player/CPlayer.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Component/CMovementComponent.h"

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
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	check(SpringArmComp);
	SpringArmComp->SetupAttachment(GetCapsuleComponent());
	SpringArmComp->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = true;

	// Init CameraComp
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	check(CameraComp);
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->SetRelativeLocation(FVector(0.0f, 40.0f, 0.0f));
	CameraComp->bUsePawnControlRotation = false;

	// Init MovementComp (Custom)
	MovementComp = CreateDefaultSubobject<UCMovementComponent>(TEXT("Movement"));
	check(MovementComp);
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
	if (IsValid(Controller) && IsValid(MovementComp))
		MovementComp->OnMoveForward(InAxisValue);
}

void ACPlayer::HandleMoveRight(const float InAxisValue)
{
	if (IsValid(Controller) && IsValid(MovementComp))
		MovementComp->OnMoveRight(InAxisValue);
}

void ACPlayer::HandleWalk()
{
	if (IsValid(Controller) && IsValid(MovementComp))
		MovementComp->OnWalk();
}

void ACPlayer::HandleRun()
{
	if (IsValid(Controller) && IsValid(MovementComp))
		MovementComp->OnRun();
}