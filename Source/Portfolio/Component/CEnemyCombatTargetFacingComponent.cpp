#include "Component/CEnemyCombatTargetFacingComponent.h"

#include "Component/CCombatTargetComponent.h"
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

void UCEnemyCombatTargetFacingComponent::InitializeReferences(const FCharacterComponentReferences& InReferences)
{
	CancelQueuedCombatTargetFacingSync();
	bCombatTargetFacingSyncPending = false;

	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.RemoveAll(this);
	}
	if (IsValid(ReactionComponent_Injected))
	{
		ReactionComponent_Injected->OnReactionExecutionLifecycleEvent.RemoveAll(this);
	}

	CombatTargetComponent_Injected = InReferences.CombatTargetComponent;
	MovementComponent_Injected = InReferences.MovementComponent;
	ReactionComponent_Injected = InReferences.ReactionComponent;

	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.AddUObject(this, &UCEnemyCombatTargetFacingComponent::HandleCombatTargetChanged);
	}
	if (IsValid(ReactionComponent_Injected))
	{
		ReactionComponent_Injected->OnReactionExecutionLifecycleEvent.AddUObject(this, &UCEnemyCombatTargetFacingComponent::HandleReactionExecutionLifecycleEvent);
	}

	SynchronizeCombatTargetFacing();
}

void UCEnemyCombatTargetFacingComponent::SetAIController(AAIController* InAIController)
{
	if (AIController_Injected == InAIController)
	{
		SynchronizeCombatTargetFacing();
		return;
	}

	ClearCombatTargetFacing();
	AIController_Injected = InAIController;
	SynchronizeCombatTargetFacing();
}

void UCEnemyCombatTargetFacingComponent::ClearAIController()
{
	ClearCombatTargetFacing();
	AIController_Injected = nullptr;
}

void UCEnemyCombatTargetFacingComponent::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	CancelQueuedCombatTargetFacingSync();
	bCombatTargetFacingSyncPending = false;

	if (IsValid(CombatTargetComponent_Injected))
	{
		CombatTargetComponent_Injected->OnCombatTargetChanged.RemoveAll(this);
	}
	if (IsValid(ReactionComponent_Injected))
	{
		ReactionComponent_Injected->OnReactionExecutionLifecycleEvent.RemoveAll(this);
	}

	ClearAIController();
	CombatTargetComponent_Injected = nullptr;
	MovementComponent_Injected = nullptr;
	ReactionComponent_Injected = nullptr;

	Super::EndPlay(InEndPlayReason);
}

void UCEnemyCombatTargetFacingComponent::HandleCombatTargetChanged(const FCombatTargetChange& InChange)
{
	ApplyCombatTargetFacing(InChange.CurrentSnapshot);
}

void UCEnemyCombatTargetFacingComponent::HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent)
{
	if (InEvent.EventType != EReactionExecutionLifecycleEventType::Completed
		&& InEvent.EventType != EReactionExecutionLifecycleEventType::Interrupted
		&& InEvent.EventType != EReactionExecutionLifecycleEventType::Ignored) return;
	QueueCombatTargetFacingSync();
}

void UCEnemyCombatTargetFacingComponent::QueueCombatTargetFacingSync()
{
	if (!bCombatTargetFacingSyncPending || bCombatTargetFacingSyncQueued) return;

	UWorld* world = GetWorld();
	if (!IsValid(world)) return;

	bCombatTargetFacingSyncQueued = true;
	CombatTargetFacingSyncTimerHandle = world->GetTimerManager().SetTimerForNextTick(this, &UCEnemyCombatTargetFacingComponent::ResolveQueuedCombatTargetFacingSync);
}

void UCEnemyCombatTargetFacingComponent::ResolveQueuedCombatTargetFacingSync()
{
	bCombatTargetFacingSyncQueued = false;
	CombatTargetFacingSyncTimerHandle.Invalidate();

	if (!bCombatTargetFacingSyncPending || ShouldDeferCombatTargetFacing()) return;

	bCombatTargetFacingSyncPending = false;
	SynchronizeCombatTargetFacing();
}

void UCEnemyCombatTargetFacingComponent::CancelQueuedCombatTargetFacingSync()
{
	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().ClearTimer(CombatTargetFacingSyncTimerHandle);
	}

	bCombatTargetFacingSyncQueued = false;
	CombatTargetFacingSyncTimerHandle.Invalidate();
}

void UCEnemyCombatTargetFacingComponent::SynchronizeCombatTargetFacing()
{
	const FCombatTargetSnapshot snapshot = IsValid(CombatTargetComponent_Injected) ? CombatTargetComponent_Injected->GetCombatTargetSnapshot() : FCombatTargetSnapshot();
	ApplyCombatTargetFacing(snapshot);
}

bool UCEnemyCombatTargetFacingComponent::ShouldDeferCombatTargetFacing() const
{
	return IsValid(ReactionComponent_Injected) && ReactionComponent_Injected->IsActive();
}

void UCEnemyCombatTargetFacingComponent::ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot)
{
	if (ShouldDeferCombatTargetFacing())
	{
		bCombatTargetFacingSyncPending = true;
		return;
	}
	bCombatTargetFacingSyncPending = false;

	if (!IsValid(AIController_Injected) || !IsValid(InSnapshot.TargetActor))
	{
		ClearCombatTargetFacing();
		return;
	}

	AIController_Injected->SetFocus(InSnapshot.TargetActor, EAIFocusPriority::Gameplay);
	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::ControllerDesired);
	}
}

void UCEnemyCombatTargetFacingComponent::ClearCombatTargetFacing()
{
	if (IsValid(AIController_Injected))
	{
		AIController_Injected->ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (IsValid(MovementComponent_Injected))
	{
		MovementComponent_Injected->SetMovementRotationMode(EMovementRotationMode::OrientToMovement);
	}
}
