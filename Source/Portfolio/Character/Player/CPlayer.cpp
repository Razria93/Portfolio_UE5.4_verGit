#include "Character/Player/CPlayer.h"

#include "ProjectGlobal.h"

#include "Core/Debug/FCombatResultDebug.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CDefenseComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CCombatTargetComponent.h"
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
#include "Type/CActionTypes.h"
#include "Type/CReactionTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CStateTypes.h"
#include "Type/CActionOrchestrationTypes.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

namespace
{
	namespace PlayerCombatDefaults
	{
		constexpr int32 MinimumParryStaggerThreshold = 1;
	}
}

ACPlayer::ACPlayer()
{
	UCharacterMovementComponent* characterMovementComp = GetCharacterMovement();
	check(characterMovementComp);
	characterMovementComp->bOrientRotationToMovement = true;

	bUseControllerRotationYaw = false;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	check(SpringArmComponent);
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	check(CameraComponent);
	CameraComponent->SetupAttachment(SpringArmComponent);
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

	CombatTargetComponent = CreateDefaultSubobject<UCCombatTargetComponent>(TEXT("CombatTarget"));
	check(CombatTargetComponent);

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

	ApplyCharacterSetup();
}

// Lifecycle

void ACPlayer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyCharacterSetup();
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
}

void ACPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// Setup

void ACPlayer::ApplyCharacterSetup()
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	check(CapsuleComp);
	CapsuleComp->InitCapsuleSize(CapsuleSetup.Radius, CapsuleSetup.HalfHeight);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);
	MeshComp->SetRelativeLocation(MeshSetup.RelativeLocation);
	MeshComp->SetRelativeRotation(MeshSetup.RelativeRotation);

	UCharacterMovementComponent* characterMovementComp = GetCharacterMovement();
	check(characterMovementComp);
	characterMovementComp->MaxWalkSpeed = MovementSetup.DefaultWalkSpeed;

	if (SpringArmComponent)
	{
		SpringArmComponent->SetRelativeLocation(CameraSetup.SpringArmRelativeLocation);
		SpringArmComponent->TargetArmLength = CameraSetup.BoomLength;
	}

	if (CameraComponent)
	{
		CameraComponent->SetRelativeLocation(CameraSetup.CameraRelativeLocation);
	}
}

// Component Reference

void ACPlayer::RecoverReferences()
{
	FComponentReferenceHelper::RecoverIfInvalid(this, MovementComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, WeaponComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, StateComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, HealthComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, DefenseComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ObservableOverlayComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, CombatTargetComponent);

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
	OutReferences.CombatTargetComponent = CombatTargetComponent;

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

// Input

void ACPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Damage

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

// Combat Result

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
	const int32 threshold = FMath::Max(PlayerCombatDefaults::MinimumParryStaggerThreshold, ParryStaggerThreshold);
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
	bool bStarted = result.IsAccepted();

	FCombatResultDebug::RecordParryStaggerReactionRequestedForAudit(this, InCombatResultPacket, result);

	return bStarted;
}

// Movement Intent

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

FActionRequestResult ACPlayer::HandleLocomotionGaitInput(const bool bWalkInputHeld, const bool bSprintInputHeld)
{
	if (bWalkInputHeld) return HandleWalk();

	const EMovementRotationMode rotationMode = IsValid(MovementComponent)
		? MovementComponent->GetCurrentMovementRotationMode()
		: EMovementRotationMode::None;
	if (bSprintInputHeld && rotationMode == EMovementRotationMode::OrientToMovement)
	{
		return HandleSprint();
	}

	return HandleRun();
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

// Action Intent

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
