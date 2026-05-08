#include "Character/Player/CPlayer.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Component/CActionOrchestratorComponent.h"
#include "Component/CReactionOrchestratorComponent.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CApplyDamageComponent.h"
#include "Component/CTakeDamageComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CDamageFeedbackComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Component/CReactionFeedbackComponent.h"

#include "Action/CAction.h"

#include "Type/CWeaponStructure.h"
#include "Type/CStateStructure.h"
#include "Type/CActionOrchestrationStructure.h"

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

	// Init ActionOrchestratorComp
	ActionOrchestratorComponent = CreateDefaultSubobject<UCActionOrchestratorComponent>(TEXT("ActionOrchestrator"));
	check(ActionOrchestratorComponent);

	// Init ReactionOrchestratorComp
	ReactionOrchestratorComponent = CreateDefaultSubobject<UCReactionOrchestratorComponent>(TEXT("ReactionOrchestrator"));
	check(ReactionOrchestratorComponent);

	// Init MovementComp (Custom)
	MovementComponent = CreateDefaultSubobject<UCMovementComponent>(TEXT("Movement"));
	check(MovementComponent);

	// Init WeaponComp
	WeaponComponent = CreateDefaultSubobject<UCWeaponComponent>(TEXT("Weapon"));
	check(WeaponComponent);

	// Init StateComp
	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);

	// Init HealthComp
	HealthComponent = CreateDefaultSubobject<UCHealthComponent>(TEXT("Health"));
	check(HealthComponent);

	// Init ApplyDamageComp
	ApplyDamageComponent = CreateDefaultSubobject<UCApplyDamageComponent>(TEXT("ApplyDamage"));
	check(ApplyDamageComponent);

	// Init TakeDamageComp
	TakeDamageComponent = CreateDefaultSubobject<UCTakeDamageComponent>(TEXT("TakeDamage"));
	check(TakeDamageComponent);

	// Init UCACtionComp
	ActionComponent = CreateDefaultSubobject<UCActionComponent>(TEXT("Action"));
	check(ActionComponent);

	// Init ReactionComp
	ReactionComponent = CreateDefaultSubobject<UCReactionComponent>(TEXT("Reaction"));
	check(ReactionComponent);

	// Init DamageFeedbackComp
	DamageFeedbackComponent = CreateDefaultSubobject<UCDamageFeedbackComponent>(TEXT("DamageFeedback"));
	check(DamageFeedbackComponent);

	// Init ActionFeedbackComp
	ActionFeedbackComponent = CreateDefaultSubobject<UCActionFeedbackComponent>(TEXT("ActionFeedback"));
	check(ActionFeedbackComponent);

	// Init ReactionFeedbackComp
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

FActionRequestResult ACPlayer::HandleMove(const FVector2D& InAxis2D)
{
	if (!IsValid(Controller) || !IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = EMovementActionIntent::Move;
	request.IntentEvent = EActionIntentEvent::Updated;
	request.Axis2D = InAxis2D;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACPlayer::HandleWalk()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = EMovementActionIntent::Walk;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACPlayer::HandleRun()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = EMovementActionIntent::Run;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACPlayer::HandleSprint()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = EMovementActionIntent::Sprint;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACPlayer::HandleJump()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = EMovementActionIntent::Jump;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACPlayer::HandleStopJump()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = EMovementActionIntent::StopJump;
	request.IntentEvent = EActionIntentEvent::Completed;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACPlayer::HandleEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent)
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FEquipmentActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = InEquipmentActionIntent;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestEquipmentAction(request);
}

FActionRequestResult ACPlayer::HandleCombatAction(ECombatActionIntent InCombatActionIntent)
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FCombatActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = InCombatActionIntent;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestCombatAction(request);
}
