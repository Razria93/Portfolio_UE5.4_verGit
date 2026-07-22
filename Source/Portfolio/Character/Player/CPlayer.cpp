#include "Character/Player/CPlayer.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "Core/Debug/FCombatResultDebug.h"

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
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(40.0f, 90.0f);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // FRotator: (Pitch, Yaw, Roll)

	UCharacterMovementComponent* characterMovementComp = GetCharacterMovement();
	check(characterMovementComp);
	characterMovementComp->bOrientRotationToMovement = true;
	characterMovementComp->MaxWalkSpeed = 600.0f;

	bUseControllerRotationYaw = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	check(SpringArmComponent);
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 55.0f));
	SpringArmComponent->TargetArmLength = 300.0f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	check(CameraComponent);
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(0.0f, 40.0f, 0.0f));
	CameraComponent->bUsePawnControlRotation = false;

	MovementComponent = CreateDefaultSubobject<UCMovementComponent>(TEXT("Movement"));
	check(MovementComponent);

	WeaponComponent = CreateDefaultSubobject<UCWeaponComponent>(TEXT("Weapon"));
	check(WeaponComponent);

	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);

	HealthComponent = CreateDefaultSubobject<UCHealthComponent>(TEXT("Health"));
	check(HealthComponent);

	DefenseComponent = CreateDefaultSubobject<UCDefenseComponent>(TEXT("Defense"));
	check(DefenseComponent);

	ObservableOverlayComponent = CreateDefaultSubobject<UCObservableOverlayComponent>(TEXT("ObservableOverlay"));
	check(ObservableOverlayComponent);

	CombatSignalSourceComponent = CreateDefaultSubobject<UCCombatSignalSourceComponent>(TEXT("CombatSignalSource"));
	check(CombatSignalSourceComponent);

	CombatSignalTargetComponent = CreateDefaultSubobject<UCCombatSignalTargetComponent>(TEXT("CombatSignalTarget"));
	check(CombatSignalTargetComponent);

	ActionOrchestratorComponent = CreateDefaultSubobject<UCActionOrchestratorComponent>(TEXT("ActionOrchestrator"));
	check(ActionOrchestratorComponent);

	ReactionOrchestratorComponent = CreateDefaultSubobject<UCReactionOrchestratorComponent>(TEXT("ReactionOrchestrator"));
	check(ReactionOrchestratorComponent);

	ActionComponent = CreateDefaultSubobject<UCActionComponent>(TEXT("Action"));
	check(ActionComponent);

	ReactionComponent = CreateDefaultSubobject<UCReactionComponent>(TEXT("Reaction"));
	check(ReactionComponent);

	HitFeedbackComponent = CreateDefaultSubobject<UCHitFeedbackComponent>(TEXT("HitFeedback"));
	check(HitFeedbackComponent);

	ActionFeedbackComponent = CreateDefaultSubobject<UCActionFeedbackComponent>(TEXT("ActionFeedback"));
	check(ActionFeedbackComponent);

	ReactionFeedbackComponent = CreateDefaultSubobject<UCReactionFeedbackComponent>(TEXT("ReactionFeedback"));
	check(ReactionFeedbackComponent);
}

void ACPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RecoverReferences();

	FCharacterComponentReferences references;
	BuildReferences(references);
	InjectReferences(references);
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

void ACPlayer::RecoverReferences()
{
	FComponentReferenceHelper::RecoverIfInvalid(this, MovementComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, WeaponComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, StateComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, HealthComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, DefenseComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ObservableOverlayComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, CombatSignalSourceComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, CombatSignalTargetComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, ActionOrchestratorComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ReactionOrchestratorComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, ActionComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ReactionComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, HitFeedbackComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ActionFeedbackComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ReactionFeedbackComponent);
}

void ACPlayer::BuildReferences(FCharacterComponentReferences& OutReferences)
{
	OutReferences.OwnerCharacter = this;

	OutReferences.MovementComponent = MovementComponent;
	OutReferences.WeaponComponent = WeaponComponent;
	OutReferences.StateComponent = StateComponent;
	OutReferences.HealthComponent = HealthComponent;
	OutReferences.DefenseComponent = DefenseComponent;
	OutReferences.ObservableOverlayComponent = ObservableOverlayComponent;

	OutReferences.CombatSignalSourceComponent = CombatSignalSourceComponent;
	OutReferences.CombatSignalTargetComponent = CombatSignalTargetComponent;

	OutReferences.ActionOrchestratorComponent = ActionOrchestratorComponent;
	OutReferences.ReactionOrchestratorComponent = ReactionOrchestratorComponent;

	OutReferences.ActionComponent = ActionComponent;
	OutReferences.ReactionComponent = ReactionComponent;

	OutReferences.HitFeedbackComponent = HitFeedbackComponent;
	OutReferences.ActionFeedbackComponent = ActionFeedbackComponent;
	OutReferences.ReactionFeedbackComponent = ReactionFeedbackComponent;
}

void ACPlayer::InjectReferences(const FCharacterComponentReferences& InReferences)
{
	FComponentReferenceHelper::InjectIfValid(MovementComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(WeaponComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(StateComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(HealthComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(DefenseComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ObservableOverlayComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(CombatSignalSourceComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(CombatSignalTargetComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(ActionOrchestratorComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ReactionOrchestratorComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(ActionComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ReactionComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(HitFeedbackComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ActionFeedbackComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ReactionFeedbackComponent, InReferences);
}

void ACPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ACPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.f) return 0.f;

	// TODO(Gameplay): Decide whether dead actors should bypass the engine TakeDamage route.

	float finalDamage = DamageAmount;

	if (IsValid(CombatSignalTargetComponent))
	{
		finalDamage = CombatSignalTargetComponent->RequestCombatSignalTarget(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	else
	{
		finalDamage = DamageAmount;
	}

	Super::TakeDamage(finalDamage, DamageEvent, EventInstigator, DamageCauser);

	return finalDamage;
}

void ACPlayer::ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket)
{
	FCombatResultDebug::RecordCombatResultReceivedForAudit(this, InCombatResultPacket);

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

	FCombatResultDebug::RecordParryStackUpdatedForAudit(this, InCombatResultPacket, ParryResultCount, threshold, bStaggerReady);

	if (bStaggerReady && TryRequestParryStaggerReaction(InCombatResultPacket))
	{
		ParryResultCount = 0;
	}
}

bool ACPlayer::TryRequestParryStaggerReaction(const FCombatResultPacket& InCombatResultPacket)
{
	if (!IsValid(ReactionOrchestratorComponent))
	{
		FCombatResultDebug::RecordParryStaggerReactionRejectedForAudit(this, InCombatResultPacket, TEXT("InvalidReactionOrchestrator"));
		return false;
	}

	FCombatResultReactionRequest request;
	request.IntentSource = EReactionIntentSource::CombatResult;
	request.CombatResultPacket = InCombatResultPacket;
	request.ReactionType = EReactionType::Stagger;

	const FReactionRequestResult result = ReactionOrchestratorComponent->RequestCombatResultReaction(request);
	const bool bStarted = result.IsAccepted();

	FCombatResultDebug::RecordParryStaggerReactionRequestedForAudit(this, InCombatResultPacket, result);

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
