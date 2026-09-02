#include "Component/CEnemyCombatTargetFacingComponent.h"

#include "ProjectGlobal.h"

#include "Component/CCombatTargetComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CHealthComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatTargetTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "Core/Debug/FEnemyCombatTargetFacingDebug.h"

#include "AIController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

namespace
{
	EEnemyCombatTargetFacingFocusDirective ResolveExpectedFocusDirective(const TCHAR* InDecision)
	{
		const FString decision = InDecision ? InDecision : TEXT("");
		if (decision == TEXT("AppliedTargetFacing")) return EEnemyCombatTargetFacingFocusDirective::SetCombatTarget;
		if (decision == TEXT("BoundFromOwner")) return EEnemyCombatTargetFacingFocusDirective::HoldCurrentFocus;
		if (decision == TEXT("OwnerControllerMissing")) return EEnemyCombatTargetFacingFocusDirective::ClearGameplayFocus;
		if (decision.Contains(TEXT("Deferred"))) return EEnemyCombatTargetFacingFocusDirective::HoldCurrentFocus;
		if (decision == TEXT("ExternalGameplayFocusCleared")) return EEnemyCombatTargetFacingFocusDirective::ClearGameplayFocus;
		if (decision == TEXT("NoPendingSync") || decision == TEXT("StillDeferredForReaction")) return EEnemyCombatTargetFacingFocusDirective::HoldCurrentFocus;
		return EEnemyCombatTargetFacingFocusDirective::ClearGameplayFocus;
	}

	EEnemyCombatTargetFacingRotationDirective ResolveExpectedRotationDirective(const TCHAR* InDecision)
	{
		const FString decision = InDecision ? InDecision : TEXT("");
		if (decision == TEXT("AppliedTargetFacing")) return EEnemyCombatTargetFacingRotationDirective::ControllerDesired;
		if (decision == TEXT("BoundFromOwner") || decision == TEXT("OwnerControllerMissing")) return EEnemyCombatTargetFacingRotationDirective::HoldCurrentRotation;
		if (decision.Contains(TEXT("Deferred")) || decision == TEXT("NoPendingSync") || decision == TEXT("StillDeferredForReaction"))
		{
			return EEnemyCombatTargetFacingRotationDirective::HoldCurrentRotation;
		}
		if (decision == TEXT("ExternalGameplayFocusCleared")) return EEnemyCombatTargetFacingRotationDirective::HoldCurrentRotation;
		return EEnemyCombatTargetFacingRotationDirective::OrientToMovement;
	}
}

UCEnemyCombatTargetFacingComponent::UCEnemyCombatTargetFacingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Component Reference

void UCEnemyCombatTargetFacingComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	CancelDeferredCombatTargetFacingSync();
	bDeferredCombatTargetFacingSyncPending = false;
	UnbindCombatTargetFacingEvents();

	CombatTargetComponent_Injected = InReferences.CombatTargetComponent;
	MovementComponent_Injected = InReferences.MovementComponent;
	ReactionComponent_Injected = InReferences.ReactionComponent;
	BalanceComponent_Injected = InReferences.BalanceComponent;
	HealthComponent_Injected = InReferences.HealthComponent;

	BindCombatTargetFacingEvents();
	ValidateRequiredComponentReferences();
	SynchronizeAIControllerBindingFromOwner(TEXT("InitializeReferences"));

	SynchronizeCombatTargetFacing(TEXT("InitializeReferences"));
}

// AI Controller Binding

void UCEnemyCombatTargetFacingComponent::SetAIController(AAIController* InAIController)
{
	if (AIController_Bound == InAIController)
	{
		SynchronizeCombatTargetFacing(TEXT("SetAIControllerSame"));
		return;
	}

	ClearCombatTargetFacing(TEXT("SetAIController"), TEXT("ClearPreviousController"));
	AIController_Bound = InAIController;
	SynchronizeCombatTargetFacing(TEXT("SetAIController"));
}

void UCEnemyCombatTargetFacingComponent::ClearAIController()
{
	const EMovementRotationMode previousRotationMode = IsValid(MovementComponent_Injected)
		? MovementComponent_Injected->GetCurrentMovementRotationMode()
		: EMovementRotationMode::None;
	AActor* previousGameplayFocusActor = IsValid(AIController_Bound)
		? AIController_Bound->GetFocusActorForPriority(EAIFocusPriority::Gameplay)
		: nullptr;

	if (IsValid(AIController_Bound))
	{
		AIController_Bound->ClearFocus(EAIFocusPriority::Gameplay);
	}
	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::OrientToMovement);
	}

	AIController_Bound = nullptr;
	RecordCombatTargetFacingDecision(TEXT("ClearAIController"), TEXT("ControllerUnbound"), previousRotationMode, previousGameplayFocusActor);
}

void UCEnemyCombatTargetFacingComponent::ClearGameplayFocusFromExternal(AAIController* InAIController, const TCHAR* InSource)
{
	AAIController* controller = IsValid(InAIController) ? InAIController : AIController_Bound;
	AActor* previousFocusActor = IsValid(controller)
		? controller->GetFocusActorForPriority(EAIFocusPriority::Gameplay)
		: nullptr;
	const EMovementRotationMode previousRotationMode = IsValid(MovementComponent_Injected)
		? MovementComponent_Injected->GetCurrentMovementRotationMode()
		: EMovementRotationMode::None;

	if (IsValid(controller))
	{
		controller->ClearFocus(EAIFocusPriority::Gameplay);
	}

	RecordCombatTargetFacingDecision(
		InSource ? InSource : TEXT("ExternalGameplayFocusClear"),
		TEXT("ExternalGameplayFocusCleared"),
		previousRotationMode,
		previousFocusActor);
}

FEnemyCombatTargetFacingRuntimeSnapshot UCEnemyCombatTargetFacingComponent::GetRuntimeSnapshot() const
{
	FEnemyCombatTargetFacingRuntimeSnapshot snapshot;
	snapshot.bHasComponent = true;

	const FCombatTargetSnapshot combatTargetSnapshot = IsValid(CombatTargetComponent_Injected)
		? CombatTargetComponent_Injected->GetCombatTargetSnapshot()
		: FCombatTargetSnapshot();
	snapshot.CombatTargetActor = combatTargetSnapshot.TargetActor;
	snapshot.CombatTargetRevision = combatTargetSnapshot.Revision;
	snapshot.CombatTargetChangeReason = combatTargetSnapshot.LastChangeReason;
	snapshot.OwnerAIController = ResolveOwnerAIController();
	snapshot.BoundAIController = AIController_Bound;
	snapshot.GameplayFocusActor = IsValid(AIController_Bound)
		? AIController_Bound->GetFocusActorForPriority(EAIFocusPriority::Gameplay)
		: nullptr;
	snapshot.RotationMode = IsValid(MovementComponent_Injected)
		? MovementComponent_Injected->GetCurrentMovementRotationMode()
		: EMovementRotationMode::None;
	snapshot.bIsReactionActive = IsValid(ReactionComponent_Injected) && ReactionComponent_Injected->IsActive();
	snapshot.ActiveReactionType = snapshot.bIsReactionActive
		? ReactionComponent_Injected->GetActiveReactionType()
		: EReactionType::None;
	snapshot.bIsDead = IsValid(HealthComponent_Injected) && HealthComponent_Injected->IsDead();
	snapshot.bIsBalanceSuppressed = IsValid(BalanceComponent_Injected) && BalanceComponent_Injected->ShouldSuppressCombatTargetFacing();
	snapshot.bDeferredSyncPending = bDeferredCombatTargetFacingSyncPending;
	snapshot.bDeferredSyncQueued = bDeferredCombatTargetFacingSyncQueued;
	snapshot.bControllerBindingMatchesOwner = IsValid(snapshot.OwnerAIController)
		&& snapshot.OwnerAIController == snapshot.BoundAIController;
	snapshot.LastEventName = LastFacingEventName;
	snapshot.LastDecision = LastFacingDecision;
	snapshot.ExpectedFocusDirective = LastExpectedFocusDirective;
	snapshot.ExpectedRotationDirective = LastExpectedRotationDirective;
	snapshot.LastTransitionSequence = LastTransitionSequence;
	snapshot.LastTransitionWorldTimeSeconds = LastTransitionWorldTimeSeconds;

	if (snapshot.bIsDead)
	{
		snapshot.PolicyState = EEnemyCombatTargetFacingPolicyState::SuppressedDead;
	}
	else if (snapshot.bIsBalanceSuppressed)
	{
		snapshot.PolicyState = EEnemyCombatTargetFacingPolicyState::SuppressedBalance;
	}
	else if (!IsValid(snapshot.CombatTargetActor))
	{
		snapshot.PolicyState = EEnemyCombatTargetFacingPolicyState::NoCombatTarget;
	}
	else if (!IsValid(snapshot.BoundAIController))
	{
		snapshot.PolicyState = EEnemyCombatTargetFacingPolicyState::NoAIController;
	}
	else if (snapshot.bIsReactionActive && snapshot.bDeferredSyncPending)
	{
		snapshot.PolicyState = EEnemyCombatTargetFacingPolicyState::HoldForReaction;
	}
	else
	{
		snapshot.PolicyState = EEnemyCombatTargetFacingPolicyState::Tracking;
	}

	return snapshot;
}

// Lifecycle

void UCEnemyCombatTargetFacingComponent::BeginPlay()
{
	Super::BeginPlay();
	SynchronizeAIControllerBindingFromOwner(TEXT("BeginPlay"));
}

void UCEnemyCombatTargetFacingComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	CancelDeferredCombatTargetFacingSync();
	bDeferredCombatTargetFacingSyncPending = false;
	UnbindCombatTargetFacingEvents();

	ClearAIController();
	CombatTargetComponent_Injected = nullptr;
	MovementComponent_Injected = nullptr;
	ReactionComponent_Injected = nullptr;
	BalanceComponent_Injected = nullptr;
	HealthComponent_Injected = nullptr;

	Super::EndPlay(InEndPlayReason);
}

// Component Event Binding

void UCEnemyCombatTargetFacingComponent::BindCombatTargetFacingEvents()
{
	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.AddUObject(this, &UCEnemyCombatTargetFacingComponent::HandleCombatTargetChanged);
	}
	if (IsValid(ReactionComponent_Injected))
	{
		ReactionComponent_Injected->OnReactionExecutionLifecycleEvent.AddUObject(this, &UCEnemyCombatTargetFacingComponent::HandleReactionExecutionLifecycleEvent);
	}
	if (IsValid(BalanceComponent_Injected))
	{
		BalanceComponent_Injected->OnBalanceLifecycleStateChanged.AddUObject(this, &UCEnemyCombatTargetFacingComponent::HandleBalanceLifecycleStateChanged);
	}
	if (IsValid(HealthComponent_Injected))
	{
		HealthComponent_Injected->OnDeadStateChanged.AddUObject(this, &UCEnemyCombatTargetFacingComponent::HandleDeadStateChanged);
	}
}

void UCEnemyCombatTargetFacingComponent::UnbindCombatTargetFacingEvents()
{
	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.RemoveAll(this);
	}
	if (IsValid(ReactionComponent_Injected))
	{
		ReactionComponent_Injected->OnReactionExecutionLifecycleEvent.RemoveAll(this);
	}
	if (IsValid(BalanceComponent_Injected))
	{
		BalanceComponent_Injected->OnBalanceLifecycleStateChanged.RemoveAll(this);
	}
	if (IsValid(HealthComponent_Injected))
	{
		HealthComponent_Injected->OnDeadStateChanged.RemoveAll(this);
	}
}

// Event Handlers

void UCEnemyCombatTargetFacingComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	ApplyCombatTargetFacing(InChange.CurrentSnapshot, TEXT("CombatTargetChanged"));
}

void UCEnemyCombatTargetFacingComponent::HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent)
{
	if (InEvent.EventType != EReactionExecutionLifecycleEventType::Completed
		&& InEvent.EventType != EReactionExecutionLifecycleEventType::Interrupted
		&& InEvent.EventType != EReactionExecutionLifecycleEventType::Ignored) return;

	QueueDeferredCombatTargetFacingSync();
}

void UCEnemyCombatTargetFacingComponent::HandleBalanceLifecycleStateChanged(const EBalanceLifecycleState InPreviousState, const EBalanceLifecycleState InCurrentState)
{
	if (ShouldSuppressCombatTargetFacing())
	{
		SynchronizeAIControllerBindingFromOwner(TEXT("OwnerControllerBindingSync"));
		CancelDeferredCombatTargetFacingSync();
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing(TEXT("BalanceLifecycleChanged"), TEXT("Suppressed"));
		return;
	}

	SynchronizeCombatTargetFacing(TEXT("BalanceLifecycleChanged"));
}

void UCEnemyCombatTargetFacingComponent::HandleDeadStateChanged(const EDeadState InPreviousDeadState, const EDeadState InCurrentDeadState)
{
	if (InCurrentDeadState == EDeadState::Dead)
	{
		SynchronizeAIControllerBindingFromOwner(TEXT("OwnerControllerBindingSync"));
		CancelDeferredCombatTargetFacingSync();
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing(TEXT("DeadStateChanged"), TEXT("Dead"));
		return;
	}

	SynchronizeCombatTargetFacing(TEXT("DeadStateChanged"));
}

// Deferred Facing Synchronization

AAIController* UCEnemyCombatTargetFacingComponent::ResolveOwnerAIController() const
{
	const APawn* ownerPawn = Cast<APawn>(GetOwner());
	return IsValid(ownerPawn) ? Cast<AAIController>(ownerPawn->GetController()) : nullptr;
}

void UCEnemyCombatTargetFacingComponent::SynchronizeAIControllerBindingFromOwner(const TCHAR* InEvent)
{
	AAIController* ownerAIController = ResolveOwnerAIController();
	if (AIController_Bound == ownerAIController) return;

	const EMovementRotationMode previousRotationMode = IsValid(MovementComponent_Injected)
		? MovementComponent_Injected->GetCurrentMovementRotationMode()
		: EMovementRotationMode::None;
	AActor* previousGameplayFocusActor = IsValid(AIController_Bound)
		? AIController_Bound->GetFocusActorForPriority(EAIFocusPriority::Gameplay)
		: nullptr;

	if (IsValid(AIController_Bound))
	{
		AIController_Bound->ClearFocus(EAIFocusPriority::Gameplay);
	}

	AIController_Bound = ownerAIController;
	RecordCombatTargetFacingDecision(
		InEvent ? InEvent : TEXT("OwnerControllerBindingSync"),
		IsValid(ownerAIController) ? TEXT("BoundFromOwner") : TEXT("OwnerControllerMissing"),
		previousRotationMode,
		previousGameplayFocusActor);
}

void UCEnemyCombatTargetFacingComponent::QueueDeferredCombatTargetFacingSync()
{
	if (ShouldSuppressCombatTargetFacing())
	{
		bDeferredCombatTargetFacingSyncPending = false;
		RecordCombatTargetFacingDecision(TEXT("ReactionLifecycleFinished"), TEXT("DeferredSyncCancelledSuppressed"), IsValid(MovementComponent_Injected) ? MovementComponent_Injected->GetCurrentMovementRotationMode() : EMovementRotationMode::None);
		return;
	}
	if (!bDeferredCombatTargetFacingSyncPending || bDeferredCombatTargetFacingSyncQueued)
	{
		RecordCombatTargetFacingDecision(TEXT("ReactionLifecycleFinished"), TEXT("DeferredSyncNotQueued"), IsValid(MovementComponent_Injected) ? MovementComponent_Injected->GetCurrentMovementRotationMode() : EMovementRotationMode::None);
		return;
	}

	UWorld* world = GetWorld();
	if (!IsValid(world))
	{
		RecordCombatTargetFacingDecision(TEXT("ReactionLifecycleFinished"), TEXT("DeferredSyncInvalidWorld"), IsValid(MovementComponent_Injected) ? MovementComponent_Injected->GetCurrentMovementRotationMode() : EMovementRotationMode::None);
		return;
	}

	bDeferredCombatTargetFacingSyncQueued = true;
	DeferredCombatTargetFacingSyncTimerHandle = world->GetTimerManager().SetTimerForNextTick(this, &UCEnemyCombatTargetFacingComponent::ResolveDeferredCombatTargetFacingSync);
	RecordCombatTargetFacingDecision(TEXT("ReactionLifecycleFinished"), TEXT("DeferredSyncQueued"), IsValid(MovementComponent_Injected) ? MovementComponent_Injected->GetCurrentMovementRotationMode() : EMovementRotationMode::None);
}

void UCEnemyCombatTargetFacingComponent::ResolveDeferredCombatTargetFacingSync()
{
	bDeferredCombatTargetFacingSyncQueued = false;
	DeferredCombatTargetFacingSyncTimerHandle.Invalidate();

	if (ShouldSuppressCombatTargetFacing())
	{
		SynchronizeAIControllerBindingFromOwner(TEXT("OwnerControllerBindingSync"));
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing(TEXT("DeferredSyncResolved"), TEXT("Suppressed"));
		return;
	}

	if (!bDeferredCombatTargetFacingSyncPending)
	{
		RecordCombatTargetFacingDecision(TEXT("DeferredSyncResolved"), TEXT("NoPendingSync"), IsValid(MovementComponent_Injected) ? MovementComponent_Injected->GetCurrentMovementRotationMode() : EMovementRotationMode::None);
		return;
	}
	if (ShouldDeferCombatTargetFacingForReaction())
	{
		RecordCombatTargetFacingDecision(TEXT("DeferredSyncResolved"), TEXT("StillDeferredForReaction"), IsValid(MovementComponent_Injected) ? MovementComponent_Injected->GetCurrentMovementRotationMode() : EMovementRotationMode::None);
		return;
	}

	bDeferredCombatTargetFacingSyncPending = false;
	SynchronizeCombatTargetFacing(TEXT("DeferredSyncResolved"));
}

void UCEnemyCombatTargetFacingComponent::CancelDeferredCombatTargetFacingSync()
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(DeferredCombatTargetFacingSyncTimerHandle);
	}

	bDeferredCombatTargetFacingSyncQueued = false;
	DeferredCombatTargetFacingSyncTimerHandle.Invalidate();
}

// Facing Synchronization

void UCEnemyCombatTargetFacingComponent::SynchronizeCombatTargetFacing(const TCHAR* InEvent)
{
	const FCombatTargetSnapshot snapshot = IsValid(CombatTargetComponent_Injected) ? CombatTargetComponent_Injected->GetCombatTargetSnapshot() : FCombatTargetSnapshot();
	ApplyCombatTargetFacing(snapshot, InEvent);
}

void UCEnemyCombatTargetFacingComponent::ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot, const TCHAR* InEvent)
{
	SynchronizeAIControllerBindingFromOwner(TEXT("OwnerControllerBindingSync"));

	const EMovementRotationMode previousRotationMode = IsValid(MovementComponent_Injected)
		? MovementComponent_Injected->GetCurrentMovementRotationMode()
		: EMovementRotationMode::None;
	AActor* previousGameplayFocusActor = IsValid(AIController_Bound)
		? AIController_Bound->GetFocusActorForPriority(EAIFocusPriority::Gameplay)
		: nullptr;

	if (ShouldSuppressCombatTargetFacing())
	{
		CancelDeferredCombatTargetFacingSync();
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing(InEvent, TEXT("Suppressed"));
		return;
	}

	// Target removal is safety-critical. It must never be delayed by an active reaction,
	// otherwise the AI can retain an obsolete gameplay focus through a death reaction.
	if (!IsValid(InSnapshot.TargetActor))
	{
		CancelDeferredCombatTargetFacingSync();
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing(InEvent, TEXT("NoCombatTarget"));
		return;
	}

	if (ShouldDeferCombatTargetFacingForReaction())
	{
		bDeferredCombatTargetFacingSyncPending = true;
		RecordCombatTargetFacingDecision(InEvent, TEXT("DeferredForReaction"), previousRotationMode, previousGameplayFocusActor);
		return;
	}
	bDeferredCombatTargetFacingSyncPending = false;

	if (!IsValid(AIController_Bound))
	{
		ClearCombatTargetFacing(InEvent, TEXT("NoAIController"));
		return;
	}

	AIController_Bound->SetFocus(InSnapshot.TargetActor, EAIFocusPriority::Gameplay);
	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::ControllerDesired);
	}

	RecordCombatTargetFacingDecision(InEvent, TEXT("AppliedTargetFacing"), previousRotationMode, previousGameplayFocusActor);
}

void UCEnemyCombatTargetFacingComponent::ClearCombatTargetFacing(const TCHAR* InEvent, const TCHAR* InDecision)
{
	const EMovementRotationMode previousRotationMode = IsValid(MovementComponent_Injected)
		? MovementComponent_Injected->GetCurrentMovementRotationMode()
		: EMovementRotationMode::None;
	AActor* previousGameplayFocusActor = IsValid(AIController_Bound)
		? AIController_Bound->GetFocusActorForPriority(EAIFocusPriority::Gameplay)
		: nullptr;

	if (IsValid(AIController_Bound))
	{
		AIController_Bound->ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::OrientToMovement);
	}

	RecordCombatTargetFacingDecision(InEvent, InDecision, previousRotationMode, previousGameplayFocusActor);
}

void UCEnemyCombatTargetFacingComponent::RecordCombatTargetFacingDecision(const TCHAR* InEvent, const TCHAR* InDecision, const EMovementRotationMode InPreviousRotationMode, AActor* InPreviousGameplayFocusActor)
{
	LastFacingEventName = InEvent ? InEvent : TEXT("Unknown");
	LastFacingDecision = InDecision ? InDecision : TEXT("Unknown");
	LastExpectedFocusDirective = ResolveExpectedFocusDirective(InDecision);
	LastExpectedRotationDirective = ResolveExpectedRotationDirective(InDecision);
	++LastTransitionSequence;
	LastTransitionWorldTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	FEnemyCombatTargetFacingDebug::RecordFacingDecision(
		this,
		InPreviousRotationMode,
		InPreviousGameplayFocusActor,
		InEvent,
		InDecision);
}

// Facing Policy

bool UCEnemyCombatTargetFacingComponent::ShouldDeferCombatTargetFacingForReaction() const
{
	return IsValid(ReactionComponent_Injected) && ReactionComponent_Injected->IsActive();
}

bool UCEnemyCombatTargetFacingComponent::ShouldSuppressCombatTargetFacing() const
{
	return (IsValid(HealthComponent_Injected) && HealthComponent_Injected->IsDead())
		|| (IsValid(BalanceComponent_Injected) && BalanceComponent_Injected->ShouldSuppressCombatTargetFacing());
}

// Component Reference Validation

bool UCEnemyCombatTargetFacingComponent::ValidateRequiredComponentReferences() const
{
	bool bValid = true;

	const FRequiredReference requiredReferences[] =
	{
		{ GetOwner(), TEXT("AActor Owner") },
		{ CombatTargetComponent_Injected, TEXT("UCCombatTargetComponent") },
		{ MovementComponent_Injected, TEXT("UCMovementComponent") },
		{ ReactionComponent_Injected, TEXT("UCReactionComponent") },
		{ BalanceComponent_Injected, TEXT("UCBalanceComponent") },
		{ HealthComponent_Injected, TEXT("UCHealthComponent") },
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, GetOwner(), this);
	}

	return bValid;
}
