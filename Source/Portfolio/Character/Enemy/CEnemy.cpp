#include "Character/Enemy/CEnemy.h"
#include "ProjectGlobal.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "Controller/CAIController.h"

#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
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

#include "Type/CWeaponStructure.h"
#include "AI/Blackboard/CAIKey.h"

namespace
{
	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyMeshMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyMeshMode"),
		0,
		TEXT("Controls ACEnemy mesh runtime LOD mode. 0: visible, 1: hidden keep pose."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyMovementMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyMovementMode"),
		0,
		TEXT("Controls ACEnemy movement runtime LOD mode. 0: default, 1: disable movement state refresh, 2: block movement intent."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyActorTickMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyActorTickMode"),
		0,
		TEXT("Controls ACEnemy actor tick profiling mode. 0: default, 1: disable ACEnemy actor tick."),
		ECVF_Default);
}

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

	// Init HealthComp
	HealthComponent = CreateDefaultSubobject<UCHealthComponent>(TEXT("Health"));
	check(HealthComponent);

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

void ACEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	RecoverReferences();

	FCharacterComponentReferences references;
	BuildReferences(references);
	InjectReferences(references);
}

void ACEnemy::BeginPlay()
{
	Super::BeginPlay();

	UpdateRuntimeLODMeshMode();
	UpdateRuntimeLODMovementMode();
	UpdateRuntimeLODActorTickMode();

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
		// FLog::Log(TEXT("[Enemy|BeginPlay] Initial equip-action request rejected."));
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

void ACEnemy::RecoverReferences()
{
	FComponentReferenceHelper::RecoverIfInvalid(this, MovementComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, WeaponComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, StateComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, HealthComponent);
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

void ACEnemy::BuildReferences(FCharacterComponentReferences& OutReferences)
{
	OutReferences.OwnerCharacter = this;

	OutReferences.MovementComponent = MovementComponent;
	OutReferences.WeaponComponent = WeaponComponent;
	OutReferences.StateComponent = StateComponent;
	OutReferences.HealthComponent = HealthComponent;
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

void ACEnemy::InjectReferences(const FCharacterComponentReferences& InReferences)
{
	FComponentReferenceHelper::InjectIfValid(MovementComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(WeaponComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(StateComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(HealthComponent, InReferences);
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

void ACEnemy::UpdateRuntimeLODMeshMode()
{
	const int32 requestedMeshMode = FMath::Clamp(CVarAIRuntimeLODEnemyMeshMode.GetValueOnGameThread(), 0, 1);
	if (RuntimeLODMeshState.AppliedMode == requestedMeshMode) return;

	USkeletalMeshComponent* meshComp = GetMesh();
	if (!IsValid(meshComp)) return;

	if (!RuntimeLODMeshState.bOriginalStateCached)
	{
		RuntimeLODMeshState.OriginalVisibilityBasedAnimTickOption = static_cast<uint8>(meshComp->VisibilityBasedAnimTickOption);
		RuntimeLODMeshState.bOriginalStateCached = true;
	}

	switch (requestedMeshMode)
	{
	case 1:
		meshComp->SetHiddenInGame(true, false);
		meshComp->SetVisibility(false, false);
		meshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		break;

	case 0:
	default:
		meshComp->SetHiddenInGame(false, false);
		meshComp->SetVisibility(true, false);
		meshComp->VisibilityBasedAnimTickOption = static_cast<EVisibilityBasedAnimTickOption>(RuntimeLODMeshState.OriginalVisibilityBasedAnimTickOption);
		break;
	}

	RuntimeLODMeshState.AppliedMode = requestedMeshMode;
}

void ACEnemy::UpdateRuntimeLODMovementMode()
{
	const int32 requestedMovementMode = FMath::Clamp(CVarAIRuntimeLODEnemyMovementMode.GetValueOnGameThread(), 0, 2);

	CacheRuntimeLODMovementOriginalState();

	if (RuntimeLODMovementState.AppliedMode != requestedMovementMode)
	{
		ApplyRuntimeLODMovementMode(requestedMovementMode);
		RuntimeLODMovementState.AppliedMode = requestedMovementMode;
	}

	if (requestedMovementMode == 2)
	{
		BlockRuntimeLODMovementIntent();
	}
}

void ACEnemy::UpdateRuntimeLODActorTickMode()
{
	const int32 requestedActorTickMode = FMath::Clamp(CVarAIRuntimeLODEnemyActorTickMode.GetValueOnGameThread(), 0, 1);

	CacheRuntimeLODActorTickOriginalState();

	if (RuntimeLODActorTickState.AppliedMode == requestedActorTickMode) return;

	ApplyRuntimeLODActorTickMode(requestedActorTickMode);
	RuntimeLODActorTickState.AppliedMode = requestedActorTickMode;
}

void ACEnemy::CacheRuntimeLODMovementOriginalState()
{
	if (RuntimeLODMovementState.bOriginalStateCached) return;

	RuntimeLODMovementState.bOriginalMovementComponentTickEnabled = IsValid(MovementComponent) ? MovementComponent->IsComponentTickEnabled() : true;
	RuntimeLODMovementState.bOriginalStateCached = true;
}

void ACEnemy::CacheRuntimeLODActorTickOriginalState()
{
	if (RuntimeLODActorTickState.bOriginalStateCached) return;

	RuntimeLODActorTickState.bOriginalActorTickEnabled = IsActorTickEnabled();
	RuntimeLODActorTickState.bOriginalStateCached = true;
}

void ACEnemy::ApplyRuntimeLODMovementMode(int32 InMovementMode)
{
	switch (InMovementMode)
	{
	case 1:
		ApplyRuntimeLODMovementStateRefreshDisabled();
		break;

	case 2:
		ApplyRuntimeLODMovementIntentBlocked();
		break;

	case 0:
	default:
		ApplyRuntimeLODMovementDefault();
		break;
	}
}

void ACEnemy::ApplyRuntimeLODActorTickMode(int32 InActorTickMode)
{
	switch (InActorTickMode)
	{
	case 1:
		ApplyRuntimeLODActorTickDisabled();
		break;

	case 0:
	default:
		ApplyRuntimeLODActorTickDefault();
		break;
	}
}

void ACEnemy::ApplyRuntimeLODMovementDefault()
{
	RestoreRuntimeLODMovementStateRefresh();
	AllowRuntimeLODMovementIntent();
}

void ACEnemy::ApplyRuntimeLODMovementStateRefreshDisabled()
{
	AllowRuntimeLODMovementIntent();
	DisableRuntimeLODMovementStateRefresh();
}

void ACEnemy::ApplyRuntimeLODMovementIntentBlocked()
{
	RestoreRuntimeLODMovementStateRefresh();
	BlockRuntimeLODMovementIntent();
	StopRuntimeLODActiveMovement();
}

void ACEnemy::ApplyRuntimeLODActorTickDefault()
{
	RestoreRuntimeLODActorTick();
}

void ACEnemy::ApplyRuntimeLODActorTickDisabled()
{
	DisableRuntimeLODActorTick();
}

void ACEnemy::RestoreRuntimeLODMovementStateRefresh()
{
	if (!IsValid(MovementComponent)) return;

	MovementComponent->SetComponentTickEnabled(RuntimeLODMovementState.bOriginalMovementComponentTickEnabled);
}

void ACEnemy::DisableRuntimeLODMovementStateRefresh()
{
	if (!IsValid(MovementComponent)) return;

	MovementComponent->SetComponentTickEnabled(false);
}

void ACEnemy::AllowRuntimeLODMovementIntent()
{
	if (!IsValid(MovementComponent)) return;

	MovementComponent->SetMove();
}

void ACEnemy::BlockRuntimeLODMovementIntent()
{
	if (!IsValid(MovementComponent)) return;

	MovementComponent->SetStop();
}

void ACEnemy::StopRuntimeLODActiveMovement()
{
	AAIController* aiController = Cast<AAIController>(GetController());
	if (!IsValid(aiController)) return;

	aiController->StopMovement();
}

void ACEnemy::RestoreRuntimeLODActorTick()
{
	SetActorTickEnabled(RuntimeLODActorTickState.bOriginalActorTickEnabled);
}

void ACEnemy::DisableRuntimeLODActorTick()
{
	SetActorTickEnabled(false);
}

void ACEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRuntimeLODMeshMode();
	UpdateRuntimeLODMovementMode();
	UpdateRuntimeLODActorTickMode();
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

	if (IsValid(CombatSignalTargetComponent))
	{
		finalDamage = CombatSignalTargetComponent->RequestCombatSignalTarget(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	else
	{
		// FLog::Log(FString::Printf(
		// 	TEXT("[Enemy] TakeDamage Fallback | Target=%s | Damage=%.3f | Reason=InvalidCombatSignalTargetComponent"),
		// 	*GetNameSafe(this),
		// 	DamageAmount));

		// FallBack
		finalDamage = DamageAmount;
	}

	// Engine-Event Trigger
	Super::TakeDamage(finalDamage, DamageEvent, EventInstigator, DamageCauser);

	return finalDamage;
}

void ACEnemy::ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket)
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

void ACEnemy::HandleParryCombatResult(const FCombatResultPacket& InCombatResultPacket)
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

bool ACEnemy::TryRequestParryStaggerReaction(const FCombatResultPacket& InCombatResultPacket)
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
	blackboardComp->SetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName, bIsCombatAction);
}

void ACEnemy::OnActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, EActionEventType InActionEventType)
{
	switch (InActionEventType)
	{
	case EActionEventType::ReserveChainWindowOpened:
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
	if (!actionRequestResult.IsReservedResult()) return;
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
