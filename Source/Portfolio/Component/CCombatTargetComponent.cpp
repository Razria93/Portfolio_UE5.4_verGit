#include "Component/CCombatTargetComponent.h"

#include "GameFramework/Actor.h"

UCCombatTargetComponent::UCCombatTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

// ===== Lifecycle =====

void UCCombatTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseCombatTargetForOwnerEndPlay();

	Super::EndPlay(EndPlayReason);
}

void UCCombatTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ResolveStaleCombatTarget();
}

// ===== Target Command =====

bool UCCombatTargetComponent::RequestSetCombatTarget(AActor* InTarget, ECombatTargetChangeReason InReason)
{
	ResolveStaleCombatTarget();
	if (!IsValid(InTarget)) return false;

	AActor* previousTarget = CurrentTarget.Get();
	if (previousTarget == InTarget) return false;

	UnbindCombatTarget(previousTarget);
	CurrentTarget = InTarget;
	BindCombatTarget(InTarget);

	++CombatTargetRevision;
	LastChangeReason = InReason;

	SetComponentTickEnabled(true);

	BroadcastCombatTargetChanged(previousTarget);
	return true;
}

bool UCCombatTargetComponent::RequestClearCombatTarget(ECombatTargetChangeReason InReason)
{
	if (CurrentTarget.IsStale())
	{
		return CommitClearCombatTarget(nullptr, InReason);
	}

	AActor* previousTarget = CurrentTarget.Get();
	if (!IsValid(previousTarget)) return false;

	return CommitClearCombatTarget(previousTarget, InReason);
}

// ===== Target Query =====

bool UCCombatTargetComponent::HasCombatTarget() const
{
	return IsValid(CurrentTarget.Get());
}

AActor* UCCombatTargetComponent::GetCombatTargetActor() const
{
	return CurrentTarget.Get();
}

FCombatTargetSnapshot UCCombatTargetComponent::GetCombatTargetSnapshot() const
{
	FCombatTargetSnapshot snapshot;
	snapshot.TargetActor = GetCombatTargetActor();
	snapshot.Revision = CombatTargetRevision;
	snapshot.LastChangeReason = LastChangeReason;
	return snapshot;
}

// ===== Validation =====

bool UCCombatTargetComponent::ResolveStaleCombatTarget()
{
	if (!CurrentTarget.IsStale()) return false;

	return CommitClearCombatTarget(nullptr, ECombatTargetChangeReason::TargetEndPlay);
}

// ===== Target Lifecycle =====

void UCCombatTargetComponent::BindCombatTarget(AActor* InTarget)
{
	if (!IsValid(InTarget)) return;

	InTarget->OnEndPlay.AddUniqueDynamic(this, &UCCombatTargetComponent::HandleCombatTargetEndPlay);
}

void UCCombatTargetComponent::UnbindCombatTarget(AActor* InTarget)
{
	if (!IsValid(InTarget)) return;

	InTarget->OnEndPlay.RemoveDynamic(this, &UCCombatTargetComponent::HandleCombatTargetEndPlay);
}

void UCCombatTargetComponent::ReleaseCombatTargetForOwnerEndPlay()
{
	UnbindCombatTarget(CurrentTarget.Get());
	CurrentTarget.Reset();

	SetComponentTickEnabled(false);
}

void UCCombatTargetComponent::HandleCombatTargetEndPlay(AActor* InActor, EEndPlayReason::Type InEndPlayReason)
{
	if (!CurrentTarget.HasSameIndexAndSerialNumber(InActor)) return;

	CommitClearCombatTarget(InActor, ECombatTargetChangeReason::TargetEndPlay);
}

// ===== Target State =====

void UCCombatTargetComponent::BroadcastCombatTargetChanged(AActor* InPreviousTarget)
{
	FCombatTargetChange change;
	change.PreviousTarget = InPreviousTarget;
	change.CurrentSnapshot = GetCombatTargetSnapshot();

	OnCombatTargetChanged.Broadcast(change);
}

bool UCCombatTargetComponent::CommitClearCombatTarget(AActor* InPreviousTarget, ECombatTargetChangeReason InReason)
{
	UnbindCombatTarget(InPreviousTarget);
	CurrentTarget.Reset();
	++CombatTargetRevision;
	LastChangeReason = InReason;

	SetComponentTickEnabled(false);

	BroadcastCombatTargetChanged(InPreviousTarget);

	return true;
}
