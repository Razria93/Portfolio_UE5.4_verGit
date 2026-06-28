#include "Character/Player/CPlayer.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CHitFeedbackComponent.h"
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

	// Init DefenseComp
	DefenseComponent = CreateDefaultSubobject<UCDefenseComponent>(TEXT("Defense"));
	check(DefenseComponent);

	// Init ObservableOverlayComp
	ObservableOverlayComponent = CreateDefaultSubobject<UCObservableOverlayComponent>(TEXT("ObservableOverlay"));
	check(ObservableOverlayComponent);

	// Init CombatSignalSourceComp
	CombatSignalSourceComponent = CreateDefaultSubobject<UCCombatSignalSourceComponent>(TEXT("CombatSignalSource"));
	check(CombatSignalSourceComponent);

	// Init CombatSignalTargetComp
	CombatSignalTargetComponent = CreateDefaultSubobject<UCCombatSignalTargetComponent>(TEXT("CombatSignalTarget"));
	check(CombatSignalTargetComponent);

	// Init ActionOrchestratorComp
	ActionOrchestratorComponent = CreateDefaultSubobject<UCActionOrchestratorComponent>(TEXT("ActionOrchestrator"));
	check(ActionOrchestratorComponent);

	// Init ReactionOrchestratorComp
	ReactionOrchestratorComponent = CreateDefaultSubobject<UCReactionOrchestratorComponent>(TEXT("ReactionOrchestrator"));
	check(ReactionOrchestratorComponent);

	// Init ActionComp
	ActionComponent = CreateDefaultSubobject<UCActionComponent>(TEXT("Action"));
	check(ActionComponent);

	// Init ReactionComp
	ReactionComponent = CreateDefaultSubobject<UCReactionComponent>(TEXT("Reaction"));
	check(ReactionComponent);

	// Init HitFeedbackComp
	HitFeedbackComponent = CreateDefaultSubobject<UCHitFeedbackComponent>(TEXT("HitFeedback"));
	check(HitFeedbackComponent);

	// Init ActionFeedbackComp
	ActionFeedbackComponent = CreateDefaultSubobject<UCActionFeedbackComponent>(TEXT("ActionFeedback"));
	check(ActionFeedbackComponent);

	// Init ReactionFeedbackComp
	ReactionFeedbackComponent = CreateDefaultSubobject<UCReactionFeedbackComponent>(TEXT("ReactionFeedback"));
	check(ReactionFeedbackComponent);
}

void ACPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	const FCharacterComponentReferences references = BuildComponentReferences();
	InjectComponentReferences(references);
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

FCharacterComponentReferences ACPlayer::BuildComponentReferences()
{
	FCharacterComponentReferences references;

	references.OwnerCharacter = this;

	references.MovementComponent = MovementComponent;
	references.WeaponComponent = WeaponComponent;
	references.StateComponent = StateComponent;
	references.HealthComponent = HealthComponent;
	references.DefenseComponent = DefenseComponent;
	references.ObservableOverlayComponent = ObservableOverlayComponent;

	references.CombatSignalSourceComponent = CombatSignalSourceComponent;
	references.CombatSignalTargetComponent = CombatSignalTargetComponent;

	references.ActionOrchestratorComponent = ActionOrchestratorComponent;
	references.ReactionOrchestratorComponent = ReactionOrchestratorComponent;

	references.ActionComponent = ActionComponent;
	references.ReactionComponent = ReactionComponent;

	references.HitFeedbackComponent = HitFeedbackComponent;
	references.ActionFeedbackComponent = ActionFeedbackComponent;
	references.ReactionFeedbackComponent = ReactionFeedbackComponent;

	return references;
}

void ACPlayer::InjectComponentReferences(const FCharacterComponentReferences& InReferences)
{
	if (IsValid(MovementComponent))
	{
		MovementComponent->InitializeReferences(InReferences);
	}

	if (IsValid(ActionOrchestratorComponent))
	{
		ActionOrchestratorComponent->InitializeReferences(InReferences);
	}

	if (IsValid(ReactionOrchestratorComponent))
	{
		ReactionOrchestratorComponent->InitializeReferences(InReferences);
	}

	if (IsValid(ActionComponent))
	{
		ActionComponent->InitializeReferences(InReferences);
	}

	if (IsValid(ReactionComponent))
	{
		ReactionComponent->InitializeReferences(InReferences);
	}
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

	if (IsValid(CombatSignalTargetComponent))
	{
		finalDamage = CombatSignalTargetComponent->RequestCombatSignalTarget(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	else
	{
		FLog::Log(FString::Printf(
			TEXT("[Player] TakeDamage Fallback | Target=%s | Damage=%.3f | Reason=InvalidCombatSignalTargetComponent"),
			*GetNameSafe(this),
			DamageAmount));

		// FallBack
		finalDamage = DamageAmount;
	}

	// Engine-Event Trigger
	Super::TakeDamage(finalDamage, DamageEvent, EventInstigator, DamageCauser);

	return finalDamage;
}

void ACPlayer::ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket)
{
	FLog::Log(FString::Printf(
		TEXT("[CombatResult] Received | Receiver=%s | Requester=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InCombatResultPacket.TargetActor)));

	FLog::Log(FString::Printf(
		TEXT("[CombatResult] Packet | Outcome=%s | Source=%s | Requester=%s | DamageCauser=%s"),
		*UEnum::GetValueAsString(InCombatResultPacket.DefenseOutcome),
		*GetNameSafe(InCombatResultPacket.SourceActor),
		*GetNameSafe(InCombatResultPacket.TargetActor),
		*GetNameSafe(InCombatResultPacket.DamageCauser)));

	if (InCombatResultPacket.IsParryResult())
	{
		HandleParryCombatResult(InCombatResultPacket);
	}
}

void ACPlayer::HandleParryCombatResult(const FCombatResultPacket& InCombatResultPacket)
{
	const int32 threshold = FMath::Max(1, ParryStaggerThreshold);
	ParryResultCount = FMath::Min(ParryResultCount + 1, threshold);

	const bool bStaggerReady = ParryResultCount >= threshold;

	FLog::Log(FString::Printf(
		TEXT("[CombatResult] ParryStack | Receiver=%s | Requester=%s | Count=%d/%d | StaggerReady=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InCombatResultPacket.TargetActor),
		ParryResultCount,
		threshold,
		bStaggerReady ? TEXT("true") : TEXT("false")));

	if (bStaggerReady && TryRequestParryStaggerReaction(InCombatResultPacket))
	{
		ParryResultCount = 0;
	}
}

bool ACPlayer::TryRequestParryStaggerReaction(const FCombatResultPacket& InCombatResultPacket)
{
	if (!IsValid(ReactionOrchestratorComponent)) return false;

	FCombatResultReactionRequest request;
	request.IntentSource = EReactionIntentSource::CombatResult;
	request.CombatResultPacket = InCombatResultPacket;
	request.ReactionType = EReactionType::Stagger;

	const FReactionRequestResult result = ReactionOrchestratorComponent->RequestCombatResultReaction(request);
	const bool bStarted = result.IsAccepted();

	FLog::Log(FString::Printf(
		TEXT("[CombatResult] StaggerRequest | Receiver=%s | Requester=%s | Result=%s"),
		*GetNameSafe(this),
		*GetNameSafe(InCombatResultPacket.TargetActor),
		bStarted ? TEXT("Accepted") : TEXT("Rejected")));

	return bStarted;
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

FActionRequestResult ACPlayer::HandleCombatAction(ECombatActionIntent InCombatActionIntent, EActionIntentEvent InIntentEvent)
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FCombatActionRequest request;
	request.IntentSource = EActionIntentSource::PlayerInput;
	request.IntentType = InCombatActionIntent;
	request.IntentEvent = InIntentEvent;

	return ActionOrchestratorComponent->RequestCombatAction(request);
}
