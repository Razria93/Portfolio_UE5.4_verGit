#include "Character/Enemy/CEnemy.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "Controller/CAIController.h"

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

	if (IsValid(ActionComponent))
	{
		// Update blackboard
		ActionComponent->OnActionTypeChanged.AddDynamic(this, &ACEnemy::OnActionTypeChanged);
		ActionComponent->OnActionEvent.AddDynamic(this, &ACEnemy::OnActionEvent);
	}

	if (IsValid(HealthComponent) && IsValid(StateComponent))
	{
		HealthComponent->OnDeadStateChanged.AddUObject(StateComponent, &UCStateComponent::OnDeadStateChanged);
	}

	const FActionRequestResult actionRequestResult = HandleAIEquipmentAction(EEquipmentActionIntent::Equip);
	if (!actionRequestResult.IsAccepted())
	{
		FLog::Log(TEXT("[Enemy|BeginPlay] Initial equip-action request rejected."));
	}
}

void ACEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(ActionComponent))
	{
		ActionComponent->OnActionTypeChanged.RemoveAll(this);
		ActionComponent->OnActionEvent.RemoveAll(this);
	}

	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDeadStateChanged.RemoveAll(StateComponent);
	}

	Super::EndPlay(EndPlayReason);
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

FActionRequestResult ACEnemy::HandleAIWalk()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = EMovementActionIntent::Walk;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACEnemy::HandleAIRun()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = EMovementActionIntent::Run;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACEnemy::HandleAISprint()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = EMovementActionIntent::Sprint;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACEnemy::HandleAIJump()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = EMovementActionIntent::Jump;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACEnemy::HandleAIStopJump()
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FMovementActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = EMovementActionIntent::StopJump;
	request.IntentEvent = EActionIntentEvent::Completed;

	return ActionOrchestratorComponent->RequestMovementAction(request);
}

FActionRequestResult ACEnemy::HandleAIEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent)
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FEquipmentActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = InEquipmentActionIntent;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestEquipmentAction(request);
}

FActionRequestResult ACEnemy::HandleAICombatAction(ECombatActionIntent InCombatActionIntent)
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FCombatActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = InCombatActionIntent;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestCombatAction(request);
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

bool ACEnemy::IsCombatActionType(EActionType InActionType) const
{
	switch (InActionType)
	{
	case EActionType::ComboAttack:
		return true;


	default:
		return false; // Idle / Equip / Unequip etc..
	}
}

void ACEnemy::OnActionTypeChanged(ACharacter* InOwnerCharacter, EActionType InPreviousActionType, EActionType InNewActionType)
{
	ACAIController* aiController = Cast<ACAIController>(GetController());
	if (!IsValid(aiController)) return;

	UBlackboardComponent* blackboardComp = aiController->GetBlackboardComponent();
	if (!IsValid(blackboardComp)) return;

	const bool bIsCombatAction = IsCombatActionType(InNewActionType);
	blackboardComp->SetValueAsBool(CAIKey::Engage::bIsCombatAction, bIsCombatAction);
}

void ACEnemy::OnActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, EActionEventType InActionEventType)
{
	switch (InActionEventType)
	{
	case EActionEventType::ChainWindowOpened:
	{
		RequestChainCombatAction(InActionType, InActionIndex);
		break;
	}

	default:
		break;
	}
}

// Request API (ActionData -> Intent -> Handle)
void ACEnemy::RequestChainCombatAction(EActionType InActionType, int32 InActionIndex)
{
	const ECombatActionIntent combatActionIntent = ResolveChainCombatIntent(InActionType, InActionIndex);
	if (combatActionIntent == ECombatActionIntent::None) return;

	const FActionRequestResult actionRequestResult = HandleAICombatAction(combatActionIntent);
	if (!actionRequestResult.IsAccepted() || actionRequestResult.ResultType != EActionRequestResultType::Chained) return;
}

// Mapping API (ActionData -> Intent)
ECombatActionIntent ACEnemy::ResolveChainCombatIntent(EActionType InActionType, int32 InActionIndex) const
{
	// TODO: Use InActionIndex when ai combo branch

	switch (InActionType)
	{
	case EActionType::ComboAttack:
		return ECombatActionIntent::ComboAttack;

	default:
		return ECombatActionIntent::None;
	}
}
