#include "Character/Enemy/CEnemy.h"

#include "ProjectGlobal.h"

#include "Controller/CAIController.h"
#include "Core/Debug/FCombatResultDebug.h"
#include "Core/Debug/FDeathLifecycleDebug.h"
#include "Component/CMovementComponent.h"
#include "Component/CWeaponComponent.h"
#include "Component/CStateComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CObservableOverlayComponent.h"
#include "Component/CCombatTargetComponent.h"
#include "Component/CExecutionCollaborationComponent.h"
#include "Component/CEnemyCombatTargetFacingComponent.h"
#include "Component/CEnemyCombatParticipationComponent.h"
#include "Component/CEnemyHitReactiveComponent.h"
#include "Component/CCombatSignalSourceComponent.h"
#include "Component/CCombatSignalTargetComponent.h"
#include "Component/CActionOrchestratorComponent.h"
#include "Component/CReactionOrchestratorComponent.h"
#include "Component/CActionComponent.h"
#include "Component/CReactionComponent.h"
#include "Component/CHitFeedbackComponent.h"
#include "Component/CActionFeedbackComponent.h"
#include "Component/CReactionFeedbackComponent.h"
#include "Component/CCharacterFeedbackComponent.h"
#include "Type/CActionTypes.h"
#include "Type/CReactionTypes.h"
#include "Type/CCombatResultTypes.h"
#include "AI/Blackboard/CAIKey.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HAL/IConsoleManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TimerManager.h"

namespace
{
	enum class EEnemyMeshRuntimeLODMode : int32
	{
		Default = 0,
		HiddenKeepPose = 1,
	};

	enum class EEnemyActorTickRuntimeLODMode : int32
	{
		Default = 0,
		Disabled = 1,
	};

	constexpr int32 ToEnemyMeshRuntimeLODModeValue(EEnemyMeshRuntimeLODMode InMode)
	{
		return static_cast<int32>(InMode);
	}

	constexpr int32 ToEnemyActorTickRuntimeLODModeValue(EEnemyActorTickRuntimeLODMode InMode)
	{
		return static_cast<int32>(InMode);
	}

	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyMeshMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyMeshMode"),
		ToEnemyMeshRuntimeLODModeValue(EEnemyMeshRuntimeLODMode::Default),
		TEXT("Controls ACEnemy mesh runtime LOD mode. 0: visible, 1: hidden keep pose."),
		ECVF_Default);

	TAutoConsoleVariable<int32> CVarAIRuntimeLODEnemyActorTickMode(
		TEXT("Portfolio.AI.RuntimeLOD.EnemyActorTickMode"),
		ToEnemyActorTickRuntimeLODModeValue(EEnemyActorTickRuntimeLODMode::Default),
		TEXT("Controls ACEnemy actor tick Runtime LOD mode. 0: default, 1: disable ACEnemy actor tick."),
		ECVF_Default);
}

ACEnemy::ACEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	UCharacterMovementComponent* characterMovementComp = GetCharacterMovement();
	check(characterMovementComp);
	characterMovementComp->bOrientRotationToMovement = true;

	MovementComponent = CreateDefaultSubobject<UCMovementComponent>(TEXT("Movement"));
	check(MovementComponent);

	WeaponComponent = CreateDefaultSubobject<UCWeaponComponent>(TEXT("Weapon"));
	check(WeaponComponent);

	StateComponent = CreateDefaultSubobject<UCStateComponent>(TEXT("State"));
	check(StateComponent);

	HealthComponent = CreateDefaultSubobject<UCHealthComponent>(TEXT("Health"));
	check(HealthComponent);

	BalanceComponent = CreateDefaultSubobject<UCBalanceComponent>(TEXT("Balance"));
	check(BalanceComponent);

	ObservableOverlayComponent = CreateDefaultSubobject<UCObservableOverlayComponent>(TEXT("ObservableOverlay"));
	check(ObservableOverlayComponent);

	CombatTargetComponent = CreateDefaultSubobject<UCCombatTargetComponent>(TEXT("CombatTarget"));
	check(CombatTargetComponent);

	ExecutionCollaborationComponent = CreateDefaultSubobject<UCExecutionCollaborationComponent>(TEXT("ExecutionCollaboration"));
	check(ExecutionCollaborationComponent);

	EnemyCombatTargetFacingComponent = CreateDefaultSubobject<UCEnemyCombatTargetFacingComponent>(TEXT("EnemyCombatTargetFacing"));
	check(EnemyCombatTargetFacingComponent);

	EnemyCombatParticipationComponent = CreateDefaultSubobject<UCEnemyCombatParticipationComponent>(TEXT("EnemyCombatParticipation"));
	check(EnemyCombatParticipationComponent);

	EnemyHitReactiveComponent = CreateDefaultSubobject<UCEnemyHitReactiveComponent>(TEXT("EnemyHitReactive"));
	check(EnemyHitReactiveComponent);

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

	CharacterFeedbackComponent = CreateDefaultSubobject<UCCharacterFeedbackComponent>(TEXT("CharacterFeedback"));
	check(CharacterFeedbackComponent);

	ApplyCharacterSetup();
}

// Lifecycle

void ACEnemy::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyCharacterSetup();
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
	UpdateRuntimeLODActorTickMode();

	if (IsValid(ActionComponent))
	{
		ActionComponent->OnActionTypeChanged.AddDynamic(this, &ACEnemy::OnActionTypeChanged);
		ActionComponent->OnActionEvent.AddDynamic(this, &ACEnemy::OnActionEvent);
	}

	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDeadStateChanged.AddUObject(this, &ACEnemy::HandleOwnerDeadStateChanged);
	}

	if (IsValid(ReactionComponent))
	{
		ReactionComponent->OnReactionExecutionLifecycleEvent.AddUObject(this, &ACEnemy::HandleDeathEntryReactionLifecycleEvent);
	}

	if (IsValid(ExecutionCollaborationComponent))
	{
		ExecutionCollaborationComponent->OnExecutionLethalDeathEntryExpected.AddUObject(this, &ACEnemy::HandleExecutionLethalDeathEntryExpected);
	}

	if (IsValid(CharacterFeedbackComponent))
	{
		CharacterFeedbackComponent->OnDeathPresentationEvent.AddUObject(this, &ACEnemy::HandleDeathPresentationEvent);
	}

	if (IsValid(HealthComponent) && HealthComponent->IsDead())
	{
		BeginDeathLifecycle();
	}
	else
	{
		HandleAIEquipmentAction(EEquipmentActionIntent::Equip);
	}
}

void ACEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DeathEntryReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathFinalizeTimerHandle);

	if (IsValid(ActionComponent))
	{
		ActionComponent->OnActionTypeChanged.RemoveAll(this);
		ActionComponent->OnActionEvent.RemoveAll(this);
	}

	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDeadStateChanged.RemoveAll(this);
	}

	if (IsValid(ReactionComponent))
	{
		ReactionComponent->OnReactionExecutionLifecycleEvent.RemoveAll(this);
	}

	if (IsValid(ExecutionCollaborationComponent))
	{
		ExecutionCollaborationComponent->OnExecutionLethalDeathEntryExpected.RemoveAll(this);
	}

	if (IsValid(BalanceComponent))
	{
		BalanceComponent->ShutdownBalanceRuntime();
	}

	if (IsValid(CharacterFeedbackComponent))
	{
		CharacterFeedbackComponent->OnDeathPresentationEvent.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

// Setup

void ACEnemy::ApplyCharacterSetup()
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
}

// Component Reference

void ACEnemy::RecoverReferences()
{
	FComponentReferenceHelper::RecoverIfInvalid(this, MovementComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, WeaponComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, StateComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, HealthComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, BalanceComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ObservableOverlayComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, CombatTargetComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ExecutionCollaborationComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, EnemyCombatTargetFacingComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, EnemyCombatParticipationComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, EnemyHitReactiveComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, CombatSignalSourceComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, CombatSignalTargetComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, ActionOrchestratorComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ReactionOrchestratorComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, ActionComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ReactionComponent);

	FComponentReferenceHelper::RecoverIfInvalid(this, HitFeedbackComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ActionFeedbackComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, ReactionFeedbackComponent);
	FComponentReferenceHelper::RecoverIfInvalid(this, CharacterFeedbackComponent);
}

void ACEnemy::BuildReferences(FCharacterComponentReferences& OutReferences)
{
	OutReferences.OwnerCharacter = this;

	OutReferences.MovementComponent = MovementComponent;
	OutReferences.WeaponComponent = WeaponComponent;
	OutReferences.StateComponent = StateComponent;
	OutReferences.HealthComponent = HealthComponent;
	OutReferences.BalanceComponent = BalanceComponent;
	OutReferences.ObservableOverlayComponent = ObservableOverlayComponent;
	OutReferences.CombatTargetComponent = CombatTargetComponent;
	OutReferences.ExecutionCollaborationComponent = ExecutionCollaborationComponent;
	OutReferences.EnemyCombatTargetFacingComponent = EnemyCombatTargetFacingComponent;
	OutReferences.EnemyCombatParticipationComponent = EnemyCombatParticipationComponent;
	OutReferences.EnemyHitReactiveComponent = EnemyHitReactiveComponent;

	OutReferences.CombatSignalSourceComponent = CombatSignalSourceComponent;
	OutReferences.CombatSignalTargetComponent = CombatSignalTargetComponent;

	OutReferences.ActionOrchestratorComponent = ActionOrchestratorComponent;
	OutReferences.ReactionOrchestratorComponent = ReactionOrchestratorComponent;

	OutReferences.ActionComponent = ActionComponent;
	OutReferences.ReactionComponent = ReactionComponent;

	OutReferences.HitFeedbackComponent = HitFeedbackComponent;
	OutReferences.ActionFeedbackComponent = ActionFeedbackComponent;
	OutReferences.ReactionFeedbackComponent = ReactionFeedbackComponent;
	OutReferences.CharacterFeedbackComponent = CharacterFeedbackComponent;
}

void ACEnemy::InjectReferences(const FCharacterComponentReferences& InReferences)
{
	FComponentReferenceHelper::InjectIfValid(MovementComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(WeaponComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(StateComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(HealthComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(BalanceComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ObservableOverlayComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ExecutionCollaborationComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(EnemyCombatTargetFacingComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(EnemyCombatParticipationComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(EnemyHitReactiveComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(CombatSignalSourceComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(CombatSignalTargetComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(ActionOrchestratorComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ReactionOrchestratorComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(ActionComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ReactionComponent, InReferences);

	FComponentReferenceHelper::InjectIfValid(HitFeedbackComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ActionFeedbackComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(ReactionFeedbackComponent, InReferences);
	FComponentReferenceHelper::InjectIfValid(CharacterFeedbackComponent, InReferences);
}

// Runtime LOD

void ACEnemy::UpdateRuntimeLODMeshMode()
{
	const int32 requestedMeshMode = FMath::Clamp(
		CVarAIRuntimeLODEnemyMeshMode.GetValueOnGameThread(),
		ToEnemyMeshRuntimeLODModeValue(EEnemyMeshRuntimeLODMode::Default),
		ToEnemyMeshRuntimeLODModeValue(EEnemyMeshRuntimeLODMode::HiddenKeepPose));
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
	case ToEnemyMeshRuntimeLODModeValue(EEnemyMeshRuntimeLODMode::HiddenKeepPose):
		meshComp->SetHiddenInGame(true, false);
		meshComp->SetVisibility(false, false);
		meshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		break;

	case ToEnemyMeshRuntimeLODModeValue(EEnemyMeshRuntimeLODMode::Default):
	default:
		meshComp->SetHiddenInGame(false, false);
		meshComp->SetVisibility(true, false);
		meshComp->VisibilityBasedAnimTickOption = static_cast<EVisibilityBasedAnimTickOption>(RuntimeLODMeshState.OriginalVisibilityBasedAnimTickOption);
		break;
	}

	RuntimeLODMeshState.AppliedMode = requestedMeshMode;
}

void ACEnemy::UpdateRuntimeLODActorTickMode()
{
	const int32 requestedActorTickMode = FMath::Clamp(
		CVarAIRuntimeLODEnemyActorTickMode.GetValueOnGameThread(),
		ToEnemyActorTickRuntimeLODModeValue(EEnemyActorTickRuntimeLODMode::Default),
		ToEnemyActorTickRuntimeLODModeValue(EEnemyActorTickRuntimeLODMode::Disabled));

	CacheRuntimeLODActorTickOriginalState();

	if (RuntimeLODActorTickState.AppliedMode == requestedActorTickMode) return;

	ApplyRuntimeLODActorTickMode(requestedActorTickMode);
	RuntimeLODActorTickState.AppliedMode = requestedActorTickMode;
}

void ACEnemy::CacheRuntimeLODActorTickOriginalState()
{
	if (RuntimeLODActorTickState.bOriginalStateCached) return;

	RuntimeLODActorTickState.bOriginalActorTickEnabled = IsActorTickEnabled();
	RuntimeLODActorTickState.bOriginalStateCached = true;
}

void ACEnemy::ApplyRuntimeLODActorTickMode(int32 InActorTickMode)
{
	switch (InActorTickMode)
	{
	case ToEnemyActorTickRuntimeLODModeValue(EEnemyActorTickRuntimeLODMode::Disabled):
		ApplyRuntimeLODActorTickDisabled();
		break;

	case ToEnemyActorTickRuntimeLODModeValue(EEnemyActorTickRuntimeLODMode::Default):
	default:
		ApplyRuntimeLODActorTickDefault();
		break;
	}
}

void ACEnemy::ApplyRuntimeLODActorTickDefault()
{
	RestoreRuntimeLODActorTick();
}

void ACEnemy::ApplyRuntimeLODActorTickDisabled()
{
	DisableRuntimeLODActorTick();
}

void ACEnemy::RestoreRuntimeLODActorTick()
{
	SetActorTickEnabled(RuntimeLODActorTickState.bOriginalActorTickEnabled);
}

void ACEnemy::DisableRuntimeLODActorTick()
{
	SetActorTickEnabled(false);
}

// Tick

void ACEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateRuntimeLODMeshMode();
	UpdateRuntimeLODActorTickMode();
}

// Input

void ACEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Target Presentation Query

FVector ACEnemy::GetTargetMarkerWorldLocation() const
{
	const USkeletalMeshComponent* meshComp = GetMesh();
	if (IsValid(meshComp) && !TargetMarkerSocketName.IsNone() && meshComp->DoesSocketExist(TargetMarkerSocketName))
	{
		return meshComp->GetSocketLocation(TargetMarkerSocketName);
	}

	return GetActorLocation() + TargetMarkerFallbackOffset;
}

// Damage

float ACEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount <= 0.f) return 0.f;

	// TODO(Gameplay): Decide whether dead actors should bypass the engine TakeDamage route.

	float finalDamage = DamageAmount;

	if (IsValid(CombatSignalTargetComponent))
	{
		finalDamage = CombatSignalTargetComponent->RequestCombatDamageTarget(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	else
	{
		finalDamage = DamageAmount;
	}

	Super::TakeDamage(finalDamage, DamageEvent, EventInstigator, DamageCauser);

	return finalDamage;
}

// Combat Result

void ACEnemy::ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket)
{
	FCombatResultDebug::RecordCombatResultReceivedForAudit(this, InCombatResultPacket);
	if (!IsValid(CombatSignalTargetComponent)) return;

	CombatSignalTargetComponent->RequestCombatResultTarget(InCombatResultPacket);
}

// AI Movement Intent

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

// AI Equip Action Intent

FActionRequestResult ACEnemy::HandleAIEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent)
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	FEquipmentActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = InEquipmentActionIntent;
	request.IntentEvent = EActionIntentEvent::Started;

	return ActionOrchestratorComponent->RequestEquipmentAction(request);
}

// AI Combat Action Intent

FActionRequestResult ACEnemy::HandleAICombatAction(ECombatActionIntent InCombatActionIntent)
{
	if (!IsValid(ActionOrchestratorComponent)) return FActionRequestResult();

	UCEnemyCombatParticipationComponent* participationComp = GetEnemyCombatParticipationComp();
	FCombatTargetSnapshot targetSnapshot;
	int32 participationRevision = 0;
	if (!IsValid(participationComp) || !participationComp->TryGetCurrentEngageAssignment(targetSnapshot, participationRevision))
	{
		FActionRequestResult staleTargetResult;
		staleTargetResult.ResultType = EActionRequestResultType::Rejected;
		staleTargetResult.RejectReason = EActionRequestRejectReason::StaleCombatTarget;
		return staleTargetResult;
	}

	const uint32 actionRequestSerial = CombatActionAuthorityRuntime.AllocateRequestSerial();

	FCombatActionRequest request;
	request.IntentSource = EActionIntentSource::AI;
	request.IntentType = InCombatActionIntent;
	request.IntentEvent = EActionIntentEvent::Started;
	request.ActionRequestSerial = actionRequestSerial;

	CombatActionAuthorityRuntime.PendingTargetSnapshot = targetSnapshot;
	CombatActionAuthorityRuntime.PendingParticipationRevision = participationRevision;
	CombatActionAuthorityRuntime.PendingRequestSerial = actionRequestSerial;

	const FActionRequestResult result = ActionOrchestratorComponent->RequestCombatAction(request);

	if ((!result.IsStartedResult() && !result.IsReservedResult())
		&& CombatActionAuthorityRuntime.PendingRequestSerial == actionRequestSerial)
	{
		CombatActionAuthorityRuntime.ResetPending();
	}

	return result;
}

// Enemy Combat Action Authority Bridge

// Action Event Observation

void ACEnemy::OnActionTypeChanged(ACharacter* InOwnerCharacter, EActionType InPreviousActionType, EActionType InNewActionType)
{
	ACAIController* aiController = Cast<ACAIController>(GetController());
	const bool bIsCombatAction = IsCombatActionType(InNewActionType);
	if (IsValid(aiController))
	{
		if (UBlackboardComponent* blackboardComp = aiController->GetBlackboardComponent())
		{
			blackboardComp->SetValueAsBool(CAIKey::Engage::bIsCombatAction.KeyName, bIsCombatAction);
		}
	}

	if (!bIsCombatAction)
	{
		ReleaseCombatActionAuthority();
	}
}

void ACEnemy::OnActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, const uint32 InActionRequestSerial, EActionEventType InActionEventType)
{
	switch (InActionEventType)
	{
	case EActionEventType::ActionStarted:
	{
		if (IsCombatActionType(InActionType) && !TryAcquireCombatActionAuthority(InActionRequestSerial))
		{
			return;
		}
		break;
	}

	case EActionEventType::ActionInterrupted:
	case EActionEventType::ActionIgnored:
	{
		ReleaseCombatActionAuthority();
		break;
	}

	case EActionEventType::ReserveChainWindowOpened:
	{
		RequestChainCombatAction(InActionType, InActionIndex);
		break;
	}

	default:
		break;
	}
}

// Authority Transition

bool ACEnemy::TryAcquireCombatActionAuthority(const uint32 InActionRequestSerial)
{
	if (InActionRequestSerial == 0 || CombatActionAuthorityRuntime.PendingRequestSerial != InActionRequestSerial) return false;

	UCEnemyCombatParticipationComponent* participationComp = GetEnemyCombatParticipationComp();
	FCombatTargetSnapshot currentTargetSnapshot;
	int32 currentParticipationRevision = 0;

	if (!IsValid(participationComp)
		|| !participationComp->TryGetCurrentEngageAssignment(currentTargetSnapshot, currentParticipationRevision)
		|| currentTargetSnapshot.TargetActor != CombatActionAuthorityRuntime.PendingTargetSnapshot.TargetActor
		|| currentTargetSnapshot.Revision != CombatActionAuthorityRuntime.PendingTargetSnapshot.Revision
		|| currentParticipationRevision != CombatActionAuthorityRuntime.PendingParticipationRevision)
	{
		CombatActionAuthorityRuntime.ResetPending();
		return false;
	}

	CombatActionAuthorityRuntime.ActiveTargetSnapshot = CombatActionAuthorityRuntime.PendingTargetSnapshot;
	CombatActionAuthorityRuntime.ActiveParticipationRevision = CombatActionAuthorityRuntime.PendingParticipationRevision;
	CombatActionAuthorityRuntime.ResetPending();

	if (IsValid(EnemyCombatParticipationComponent))
	{
		EnemyCombatParticipationComponent->AcquireParticipationAssignmentLock(
			CombatActionAuthorityRuntime.ActiveTargetSnapshot,
			CombatActionAuthorityRuntime.ActiveParticipationRevision);
	}

	return true;
}

void ACEnemy::ReleaseCombatActionAuthority()
{
	if (IsValid(EnemyCombatParticipationComponent))
	{
		EnemyCombatParticipationComponent->ReleaseParticipationAssignmentLock();
	}

	CombatActionAuthorityRuntime.ResetAll();
}

// Chain Action Routing

void ACEnemy::RequestChainCombatAction(EActionType InActionType, int32 InActionIndex)
{
	const ECombatActionIntent combatActionIntent = ResolveChainCombatIntent(InActionType, InActionIndex);
	if (combatActionIntent == ECombatActionIntent::None) return;

	const FActionRequestResult actionRequestResult = HandleAICombatAction(combatActionIntent);
	if (!actionRequestResult.IsReservedResult()) return;
}

ECombatActionIntent ACEnemy::ResolveChainCombatIntent(EActionType InActionType, int32 InActionIndex) const
{
	switch (InActionType)
	{
	case EActionType::ComboAttack:
		return ECombatActionIntent::ComboAttack;

	default:
		return ECombatActionIntent::None;
	}
}

// Classification

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

// Health / Death Command

bool ACEnemy::TryStartKill()
{
	if (!IsValid(HealthComponent)) return false;

	return HealthComponent->TryKill();
}

// Death Lifecycle

// Normal:
// Health Alive -> Dead -> BeginDeathLifecycle
// -> DeadIn Started / Completed -> BeginDeathPresentation
// -> Feedback Finished -> RequestFinalizeDeath -> FinalizeDeath -> Destroy
//
// Fallback:
// DeadIn start failure / formal stop
// -> BeginDeathPresentation
// -> presentation unavailable / fallback delay
// -> RequestFinalizeDeath -> FinalizeDeath

// Query

bool ACEnemy::IsDeathPresentationFallbackPending() const
{
	const UWorld* world = GetWorld();
	return IsValid(world) && world->GetTimerManager().IsTimerActive(DeathPresentationFallbackTimerHandle);
}

// Entry / State

void ACEnemy::HandleOwnerDeadStateChanged(EDeadState InPreviousDeadState, EDeadState InNewDeadState)
{
	if (bDeathFinalized) return;

	if (InNewDeadState == EDeadState::Dead && !bDeathLifecycleActive)
	{
		BeginDeathLifecycle();
		return;
	}
}

void ACEnemy::HandleExecutionLethalDeathEntryExpected(const FExecutionSessionId& InSessionId)
{
	if (!InSessionId.IsValidMinimal()) return;

	DeathPresentationMode = EDeathPresentationMode::ExecutionLethal;
	ExpectedExecutionLethalDeathSessionId = InSessionId;
}

void ACEnemy::BeginDeathLifecycle()
{
	if (bDeathLifecycleActive || bDeathFinalized) return;

	bDeathLifecycleActive = true;
	bDeathPresentationRequested = false;
	bDeathFinalizationRequested = false;
	ApplyDeathPawnCollisionPolicy();

	if (IsValid(BalanceComponent))
	{
		BalanceComponent->AbortBalanceLifecycle(EBalanceAbortReason::OwnerDeath);
	}

	if (IsValid(EnemyCombatParticipationComponent))
	{
		EnemyCombatParticipationComponent->HardReleaseParticipationForOwnerDeath();
	}

	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("LifecycleStarted"));

	if (ACAIController* aiController = Cast<ACAIController>(GetController()))
	{
		aiController->StopMovement();
	}

	if (IsValid(ActionOrchestratorComponent))
	{
		ActionOrchestratorComponent->ClearAllDeferredActions();
	}

	if (IsValid(WeaponComponent))
	{
		WeaponComponent->ClearWeaponRuntimeState();
	}

	DeathEntryReactionStartFallbackTimerHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ACEnemy::ValidateDeathEntryReactionStarted);
}

void ACEnemy::AbortDeathLifecycle()
{
	if (bDeathFinalized) return;

	RestoreDeathPawnCollisionPolicy();
	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("LifecycleAborted"));

	GetWorldTimerManager().ClearTimer(DeathEntryReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathFinalizeTimerHandle);

	if (IsValid(CharacterFeedbackComponent))
	{
		CharacterFeedbackComponent->ClearRuntimeFeedback();
	}

	bDeathLifecycleActive = false;
	bDeathPresentationRequested = false;
	bDeathFinalizationRequested = false;
	DeathPresentationMode = EDeathPresentationMode::Default;
	ExpectedExecutionLethalDeathSessionId = FExecutionSessionId();
}

void ACEnemy::ApplyDeathPawnCollisionPolicy()
{
	if (bDeathPawnCollisionPolicyApplied) return;

	UCapsuleComponent* capsuleComp = GetCapsuleComponent();
	if (!IsValid(capsuleComp)) return;

	CachedPawnCollisionResponseBeforeDeath = capsuleComp->GetCollisionResponseToChannel(ECC_Pawn);
	capsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	bDeathPawnCollisionPolicyApplied = true;

	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PawnCollisionIgnored"));
}

void ACEnemy::RestoreDeathPawnCollisionPolicy()
{
	if (!bDeathPawnCollisionPolicyApplied) return;

	if (UCapsuleComponent* capsuleComp = GetCapsuleComponent())
	{
		capsuleComp->SetCollisionResponseToChannel(ECC_Pawn, CachedPawnCollisionResponseBeforeDeath);
	}

	bDeathPawnCollisionPolicyApplied = false;
	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PawnCollisionRestored"));
}

// Death Entry Reaction Contract

EReactionType ACEnemy::GetExpectedDeathEntryReactionType() const
{
	return DeathPresentationMode == EDeathPresentationMode::ExecutionLethal
		? EReactionType::ExecutionLethal
		: EReactionType::Dead;
}

bool ACEnemy::IsExpectedDeathEntryReaction(const FReactionExecutionContext& InContext) const
{
	if (InContext.ReactionDataKey.ReactionType != GetExpectedDeathEntryReactionType()) return false;
	return DeathPresentationMode != EDeathPresentationMode::ExecutionLethal
		|| InContext.ExecutionSessionId == ExpectedExecutionLethalDeathSessionId;
}

// Death Entry Reaction Observation

void ACEnemy::HandleDeathEntryReactionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent)
{
	if (!bDeathLifecycleActive || bDeathFinalized) return;
	if (!IsExpectedDeathEntryReaction(InEvent.Context)) return;

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Started)
	{
		GetWorldTimerManager().ClearTimer(DeathEntryReactionStartFallbackTimerHandle);
		FDeathLifecycleDebug::RecordLifecycleEvent(this, DeathPresentationMode == EDeathPresentationMode::ExecutionLethal ? TEXT("ExecutionLethalInStarted") : TEXT("DeadInStarted"));
		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed)
	{
		const bool bExecutionLethal = DeathPresentationMode == EDeathPresentationMode::ExecutionLethal;
		FDeathLifecycleDebug::RecordLifecycleEvent(this, bExecutionLethal ? TEXT("ExecutionLethalInCompleted") : TEXT("DeadInCompleted"));
		BeginDeathPresentation(bExecutionLethal
			? EDeathPresentationReason::ExecutionLethalInCompleted
			: EDeathPresentationReason::DeadInCompleted);
		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Interrupted
		|| InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored)
	{
		const bool bExecutionLethal = DeathPresentationMode == EDeathPresentationMode::ExecutionLethal;
		FDeathLifecycleDebug::RecordContractViolationForAudit(this, bExecutionLethal ? TEXT("ExecutionLethalInStopped") : TEXT("DeadInStopped"), FString::Printf(TEXT("Event: %s"), *UEnum::GetValueAsString(InEvent.EventType)));
		BeginDeathPresentation(bExecutionLethal
			? EDeathPresentationReason::ExecutionLethalInInterrupted
			: EDeathPresentationReason::DeadInInterrupted);
	}
}

// Death Entry Reaction Start Fallback

void ACEnemy::ValidateDeathEntryReactionStarted()
{
	if (!bDeathLifecycleActive || bDeathPresentationRequested || bDeathFinalizationRequested || bDeathFinalized) return;

	FReactionExecutionContext context;
	if (!IsValid(ReactionComponent) || !ReactionComponent->GetActiveReactionContext(context) || !IsExpectedDeathEntryReaction(context))
	{
		const bool bExecutionLethal = DeathPresentationMode == EDeathPresentationMode::ExecutionLethal;
		FDeathLifecycleDebug::RecordContractViolationForAudit(this, bExecutionLethal ? TEXT("ExecutionLethalInStartFailed") : TEXT("DeadInStartFailed"));
		BeginDeathPresentation(bExecutionLethal
			? EDeathPresentationReason::ExecutionLethalInStartFailed
			: EDeathPresentationReason::DeadInStartFailed);
	}
}

// Death Presentation

void ACEnemy::BeginDeathPresentation(EDeathPresentationReason InReason)
{
	if (!bDeathLifecycleActive || bDeathPresentationRequested || bDeathFinalizationRequested || bDeathFinalized) return;
	if (!IsValid(HealthComponent) || HealthComponent->IsAlive()) return;

	bDeathPresentationRequested = true;
	GetWorldTimerManager().ClearTimer(DeathEntryReactionStartFallbackTimerHandle);

	ScheduleDeathPresentationFallback();
	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PresentationRequested"), FString::Printf(TEXT("Reason: %s"), *UEnum::GetValueAsString(InReason)));

	if (!IsValid(CharacterFeedbackComponent))
	{
		FDeathLifecycleDebug::RecordContractViolationForAudit(this, TEXT("PresentationUnavailable"), TEXT("Reason: InvalidCharacterFeedback"));
		return;
	}

	if (!CharacterFeedbackComponent->RequestDeathPresentation(InReason))
	{
		FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PresentationUnavailable"), TEXT("Reason: NoPresentationListener"));
		return;
	}
}

void ACEnemy::ScheduleDeathPresentationFallback()
{
	GetWorldTimerManager().ClearTimer(DeathPresentationFallbackTimerHandle);

	const float fallbackDelay = FMath::Max(DeathPresentationFallbackDelay, 0.f);
	if (fallbackDelay <= 0.f)
	{
		DeathPresentationFallbackTimerHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ACEnemy::HandleDeathPresentationFallbackExpired);
		return;
	}

	GetWorldTimerManager().SetTimer(
		DeathPresentationFallbackTimerHandle,
		this,
		&ACEnemy::HandleDeathPresentationFallbackExpired,
		fallbackDelay,
		false);
}

void ACEnemy::HandleDeathPresentationEvent(EDeathPresentationEventType InEventType)
{
	if (!bDeathLifecycleActive || !bDeathPresentationRequested || bDeathFinalized) return;

	switch (InEventType)
	{
	case EDeathPresentationEventType::Started:
		GetWorldTimerManager().ClearTimer(DeathPresentationFallbackTimerHandle);
		FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PresentationStarted"));
		return;

	case EDeathPresentationEventType::Unavailable:
		FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PresentationUnavailable"));
		return;

	case EDeathPresentationEventType::Finished:
		GetWorldTimerManager().ClearTimer(DeathPresentationFallbackTimerHandle);
		FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PresentationFinished"));
		RequestFinalizeDeath(EDeathFinalizeReason::PresentationCompleted);
		return;

	default:
		return;
	}
}

void ACEnemy::HandleDeathPresentationFallbackExpired()
{
	if (!bDeathLifecycleActive || !bDeathPresentationRequested || bDeathFinalizationRequested || bDeathFinalized) return;

	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("PresentationFallbackExpired"));
	RequestFinalizeDeath(EDeathFinalizeReason::PresentationFallbackExpired);
}

// Finalization Gateway

void ACEnemy::RequestFinalizeDeath(EDeathFinalizeReason InReason)
{
	if (!bDeathLifecycleActive || bDeathFinalizationRequested || bDeathFinalized) return;
	if (!IsValid(HealthComponent)) return;

	if (!HealthComponent->IsDead()) return;

	bDeathFinalizationRequested = true;
	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("FinalizeRequested"), FString::Printf(TEXT("Reason: %s"), *UEnum::GetValueAsString(InReason)));
	GetWorldTimerManager().ClearTimer(DeathEntryReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationFallbackTimerHandle);
	DeathFinalizeTimerHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ACEnemy::FinalizeDeath);
}

void ACEnemy::FinalizeDeath()
{
	bDeathFinalizationRequested = false;
	if (!bDeathLifecycleActive || bDeathFinalized) return;
	if (!IsValid(HealthComponent))
	{
		AbortDeathLifecycle();
		return;
	}

	if (!HealthComponent->IsDead())
	{
		AbortDeathLifecycle();
		return;
	}

	bDeathFinalized = true;
	FDeathLifecycleDebug::RecordLifecycleEvent(this, TEXT("Finalized"));
	GetWorldTimerManager().ClearTimer(DeathEntryReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathFinalizeTimerHandle);

	CleanupDeathGameplayRuntime();
	Destroy();
}

void ACEnemy::CleanupDeathGameplayRuntime()
{
	if (ACAIController* aiController = Cast<ACAIController>(GetController()))
	{
		aiController->StopMovement();
	}

	if (IsValid(ActionOrchestratorComponent))
	{
		ActionOrchestratorComponent->ClearAllDeferredActions();
	}

	if (IsValid(WeaponComponent))
	{
		WeaponComponent->ClearWeaponRuntimeState();
	}

	if (IsValid(CharacterFeedbackComponent))
	{
		CharacterFeedbackComponent->ClearRuntimeFeedback();
	}
}
