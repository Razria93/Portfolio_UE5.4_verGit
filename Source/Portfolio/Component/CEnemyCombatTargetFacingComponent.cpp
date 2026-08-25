#include "Component/CEnemyCombatTargetFacingComponent.h"

#include "ProjectGlobal.h"

#include "Component/CCombatTargetComponent.h"
#include "Component/CBalanceComponent.h"
#include "Component/CMovementComponent.h"
#include "Component/CReactionComponent.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCombatTargetTypes.h"
#include "Type/CReactionOrchestrationTypes.h"

#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"

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

	BindCombatTargetFacingEvents();
	ValidateRequiredComponentReferences();

	SynchronizeCombatTargetFacing();
}

// AI Controller Binding

void UCEnemyCombatTargetFacingComponent::SetAIController(AAIController* InAIController)
{
	if (AIController_Bound == InAIController)
	{
		SynchronizeCombatTargetFacing();
		return;
	}

	ClearCombatTargetFacing();
	AIController_Bound = InAIController;
	SynchronizeCombatTargetFacing();
}

void UCEnemyCombatTargetFacingComponent::ClearAIController()
{
	ClearCombatTargetFacing();
	AIController_Bound = nullptr;
}

// Lifecycle

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
}

// Event Handlers

void UCEnemyCombatTargetFacingComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	ApplyCombatTargetFacing(InChange.CurrentSnapshot);
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
	if (IsCombatTargetFacingSuppressedByBalance())
	{
		CancelDeferredCombatTargetFacingSync();
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing();
		return;
	}

	SynchronizeCombatTargetFacing();
}

// Deferred Facing Synchronization

void UCEnemyCombatTargetFacingComponent::QueueDeferredCombatTargetFacingSync()
{
	if (IsCombatTargetFacingSuppressedByBalance())
	{
		bDeferredCombatTargetFacingSyncPending = false;
		return;
	}
	if (!bDeferredCombatTargetFacingSyncPending || bDeferredCombatTargetFacingSyncQueued) return;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	bDeferredCombatTargetFacingSyncQueued = true;
	DeferredCombatTargetFacingSyncTimerHandle = world->GetTimerManager().SetTimerForNextTick(this, &UCEnemyCombatTargetFacingComponent::ResolveDeferredCombatTargetFacingSync);
}

void UCEnemyCombatTargetFacingComponent::ResolveDeferredCombatTargetFacingSync()
{
	bDeferredCombatTargetFacingSyncQueued = false;
	DeferredCombatTargetFacingSyncTimerHandle.Invalidate();

	if (IsCombatTargetFacingSuppressedByBalance())
	{
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing();
		return;
	}

	if (!bDeferredCombatTargetFacingSyncPending || ShouldDeferCombatTargetFacingForReaction()) return;

	bDeferredCombatTargetFacingSyncPending = false;
	SynchronizeCombatTargetFacing();
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

void UCEnemyCombatTargetFacingComponent::SynchronizeCombatTargetFacing()
{
	const FCombatTargetSnapshot snapshot = IsValid(CombatTargetComponent_Injected) ? CombatTargetComponent_Injected->GetCombatTargetSnapshot() : FCombatTargetSnapshot();
	ApplyCombatTargetFacing(snapshot);
}

void UCEnemyCombatTargetFacingComponent::ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot)
{
	if (IsCombatTargetFacingSuppressedByBalance())
	{
		bDeferredCombatTargetFacingSyncPending = false;
		ClearCombatTargetFacing();
		return;
	}

	if (ShouldDeferCombatTargetFacingForReaction())
	{
		bDeferredCombatTargetFacingSyncPending = true;
		return;
	}
	bDeferredCombatTargetFacingSyncPending = false;

	if (!IsValid(AIController_Bound) || !IsValid(InSnapshot.TargetActor))
	{
		ClearCombatTargetFacing();
		return;
	}

	AIController_Bound->SetFocus(InSnapshot.TargetActor, EAIFocusPriority::Gameplay);
	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::ControllerDesired);
	}
}

void UCEnemyCombatTargetFacingComponent::ClearCombatTargetFacing()
{
	if (IsValid(AIController_Bound))
	{
		AIController_Bound->ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::OrientToMovement);
	}
}

// Facing Policy

bool UCEnemyCombatTargetFacingComponent::ShouldDeferCombatTargetFacingForReaction() const
{
	return IsValid(ReactionComponent_Injected) && ReactionComponent_Injected->IsActive();
}

bool UCEnemyCombatTargetFacingComponent::IsCombatTargetFacingSuppressedByBalance() const
{
	return IsValid(BalanceComponent_Injected) && BalanceComponent_Injected->ShouldSuppressCombatTargetFacing();
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
	};

	for (const FRequiredReference& reference : requiredReferences)
	{
		bValid &= FReferenceValidation::EnsureRequiredReference(reference.Object, reference.Label, GetOwner(), this);
	}

	return bValid;
}
