#include "Character/Enemy/CEnemy.h"

#include "ProjectGlobal.h"

#include "Controller/CAIController.h"
#include "Core/Debug/FCombatResultDebug.h"
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

	namespace EnemyCombatDefaults
	{
		constexpr int32 MinimumParryStaggerThreshold = 1;
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
		ReactionComponent->OnReactionExecutionLifecycleEvent.AddUObject(this, &ACEnemy::HandleReactionExecutionLifecycleEvent);
	}

	if (IsValid(CharacterFeedbackComponent))
	{
		CharacterFeedbackComponent->OnDeathPresentationFinished.AddUObject(this, &ACEnemy::HandleDeathPresentationFinished);
	}

	HandleAIEquipmentAction(EEquipmentActionIntent::Equip);
}

void ACEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(DeadReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationWatchdogTimerHandle);
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

	if (IsValid(CharacterFeedbackComponent))
	{
		CharacterFeedbackComponent->OnDeathPresentationFinished.RemoveAll(this);
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

// Component Reference

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
	FComponentReferenceHelper::RecoverIfInvalid(this, CharacterFeedbackComponent);
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
	OutReferences.CharacterFeedbackComponent = CharacterFeedbackComponent;
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

// Damage

float ACEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
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

void ACEnemy::ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket)
{
	FCombatResultDebug::RecordCombatResultReceivedForAudit(this, InCombatResultPacket);

	if (InCombatResultPacket.IsParryResult())
	{
		HandleParryCombatResult(InCombatResultPacket);
	}
}

void ACEnemy::HandleParryCombatResult(const FCombatResultPacket& InCombatResultPacket)
{
	const int32 threshold = FMath::Max(EnemyCombatDefaults::MinimumParryStaggerThreshold, ParryStaggerThreshold);
	ParryResultCount = FMath::Min(ParryResultCount + 1, threshold);

	const bool bStaggerReady = ParryResultCount >= threshold;

	FCombatResultDebug::RecordParryStackUpdatedForAudit(this, InCombatResultPacket, ParryResultCount, threshold, bStaggerReady);

	if (bStaggerReady && TryRequestParryStaggerReaction(InCombatResultPacket))
	{
		ParryResultCount = 0;
	}
}

bool ACEnemy::TryRequestParryStaggerReaction(const FCombatResultPacket& InCombatResultPacket)
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

// AI Action Intent

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

// Runtime State

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
// -> presentation start failure / presentation watchdog
// -> RequestFinalizeDeath -> FinalizeDeath

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

void ACEnemy::BeginDeathLifecycle()
{
	if (bDeathLifecycleActive || bDeathFinalized) return;

	bDeathLifecycleActive = true;
	bDeathPresentationStarted = false;
	bDeathFinalizationRequested = false;

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

	DeadReactionStartFallbackTimerHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ACEnemy::ValidateDeadReactionStarted);
}

void ACEnemy::AbortDeathLifecycle()
{
	if (bDeathFinalized) return;

	GetWorldTimerManager().ClearTimer(DeadReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationWatchdogTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathFinalizeTimerHandle);

	if (IsValid(CharacterFeedbackComponent))
	{
		CharacterFeedbackComponent->ClearRuntimeFeedback();
	}

	bDeathLifecycleActive = false;
	bDeathPresentationStarted = false;
	bDeathFinalizationRequested = false;
}

// Dead Reaction Observation

void ACEnemy::HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent)
{
	if (!bDeathLifecycleActive || bDeathFinalized) return;
	if (InEvent.Context.ReactionDataKey.ReactionType != EReactionType::Dead) return;

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Started)
	{
		GetWorldTimerManager().ClearTimer(DeadReactionStartFallbackTimerHandle);
		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Completed)
	{
		BeginDeathPresentation(EDeathPresentationReason::DeadInCompleted);
		return;
	}

	if (InEvent.EventType == EReactionExecutionLifecycleEventType::Interrupted
		|| InEvent.EventType == EReactionExecutionLifecycleEventType::Ignored)
	{
		FLog::Log(FString::Printf(TEXT("[DeathLifecycle|DeadInStopped] Owner=%s | Event=%s"), *GetNameSafe(this), *UEnum::GetValueAsString(InEvent.EventType)));
		BeginDeathPresentation(EDeathPresentationReason::DeadInInterrupted);
	}
}

// Dead Reaction Start Fallback

void ACEnemy::ValidateDeadReactionStarted()
{
	if (!bDeathLifecycleActive || bDeathPresentationStarted || bDeathFinalizationRequested || bDeathFinalized) return;

	if (!IsValid(ReactionComponent) || !ReactionComponent->IsActiveReactionType(EReactionType::Dead))
	{
		FLog::Log(FString::Printf(TEXT("[DeathLifecycle|DeadInStartFailed] Owner=%s"), *GetNameSafe(this)));
		BeginDeathPresentation(EDeathPresentationReason::DeadInStartFailed);
	}
}

// Death Presentation

void ACEnemy::BeginDeathPresentation(EDeathPresentationReason InReason)
{
	if (!bDeathLifecycleActive || bDeathPresentationStarted || bDeathFinalizationRequested || bDeathFinalized) return;
	if (!IsValid(HealthComponent) || HealthComponent->IsAlive()) return;

	bDeathPresentationStarted = true;
	GetWorldTimerManager().ClearTimer(DeadReactionStartFallbackTimerHandle);

	if (!IsValid(CharacterFeedbackComponent))
	{
		FLog::Log(FString::Printf(TEXT("[DeathLifecycle|PresentationStartFailed] Owner=%s | Reason=InvalidCharacterFeedback"), *GetNameSafe(this)));
		RequestFinalizeDeath(EDeathFinalizeReason::PresentationStartFailed);
		return;
	}

	const FDeathPresentationStartResult result = CharacterFeedbackComponent->StartDeathPresentation(InReason);
	if (!result.bStarted)
	{
		FLog::Log(FString::Printf(TEXT("[DeathLifecycle|PresentationStartFailed] Owner=%s | Reason=NoPresentationListener"), *GetNameSafe(this)));
		RequestFinalizeDeath(EDeathFinalizeReason::PresentationStartFailed);
		return;
	}

	// A Blueprint presentation may complete synchronously while StartDeathPresentation broadcasts.
	if (bDeathFinalizationRequested || bDeathFinalized || !CharacterFeedbackComponent->IsDeathPresentationActive()) return;

	ScheduleDeathPresentationWatchdog(result.ExpectedDuration);
}

void ACEnemy::ScheduleDeathPresentationWatchdog(float InExpectedDuration)
{
	GetWorldTimerManager().ClearTimer(DeathPresentationWatchdogTimerHandle);

	const float watchdogDuration = FMath::Max(
		FMath::Max(DeathPresentationWatchdogMinimumDuration, 0.f),
		FMath::Max(InExpectedDuration, 0.f) + FMath::Max(DeathPresentationWatchdogSafetyMargin, 0.f));

	GetWorldTimerManager().SetTimer(
		DeathPresentationWatchdogTimerHandle,
		this,
		&ACEnemy::HandleDeathPresentationWatchdogExpired,
		watchdogDuration,
		false);
}

void ACEnemy::HandleDeathPresentationFinished()
{
	if (!bDeathLifecycleActive || !bDeathPresentationStarted || bDeathFinalized) return;

	GetWorldTimerManager().ClearTimer(DeathPresentationWatchdogTimerHandle);
	RequestFinalizeDeath(EDeathFinalizeReason::PresentationCompleted);
}

void ACEnemy::HandleDeathPresentationWatchdogExpired()
{
	if (!bDeathLifecycleActive || !bDeathPresentationStarted || bDeathFinalizationRequested || bDeathFinalized) return;

	FLog::Log(FString::Printf(TEXT("[DeathLifecycle|PresentationTimedOut] Owner=%s"), *GetNameSafe(this)));
	RequestFinalizeDeath(EDeathFinalizeReason::PresentationTimedOut);
}

// Finalization Gateway

void ACEnemy::RequestFinalizeDeath(EDeathFinalizeReason InReason)
{
	if (!bDeathLifecycleActive || bDeathFinalizationRequested || bDeathFinalized) return;
	if (!IsValid(HealthComponent)) return;

	if (!HealthComponent->IsDead()) return;

	bDeathFinalizationRequested = true;
	FLog::Log(FString::Printf(TEXT("[DeathLifecycle|FinalizeRequested] Owner=%s | Reason=%s"), *GetNameSafe(this), *UEnum::GetValueAsString(InReason)));
	GetWorldTimerManager().ClearTimer(DeadReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationWatchdogTimerHandle);
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
	GetWorldTimerManager().ClearTimer(DeadReactionStartFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(DeathPresentationWatchdogTimerHandle);
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

// Combat Action Query

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

// Action Event Routing

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
