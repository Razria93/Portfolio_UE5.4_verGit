#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CEnemyCombatTargetFacingComponent.generated.h"

class AAIController;
class UCCombatTargetComponent;
class UCBalanceComponent;
class UCMovementComponent;
class UCReactionComponent;
enum class EBalanceLifecycleState : uint8;
struct FCharacterComponentReferences;
struct FCombatTargetChange;
struct FCombatTargetSnapshot;
struct FReactionExecutionLifecycleEvent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCEnemyCombatTargetFacingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Construction
	UCEnemyCombatTargetFacingComponent();

private:
	// Component References
	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;

	UPROPERTY(Transient)
	UCMovementComponent* MovementComponent_Injected = nullptr;

	UPROPERTY(Transient)
	UCReactionComponent* ReactionComponent_Injected = nullptr;

	UPROPERTY(Transient)
	UCBalanceComponent* BalanceComponent_Injected = nullptr;

	// AI Controller Binding
	UPROPERTY(Transient)
	AAIController* AIController_Bound = nullptr;

	// Deferred Facing Synchronization Runtime
	bool bDeferredCombatTargetFacingSyncPending = false;
	bool bDeferredCombatTargetFacingSyncQueued = false;
	FTimerHandle DeferredCombatTargetFacingSyncTimerHandle;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

	// AI Controller Binding
	void SetAIController(AAIController* InAIController);
	void ClearAIController();

protected:
	// Lifecycle
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

private:
	// Component Event Binding
	void BindCombatTargetFacingEvents();
	void UnbindCombatTargetFacingEvents();

private:
	// Event Handlers
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);
	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);
	void HandleBalanceLifecycleStateChanged(EBalanceLifecycleState InPreviousState, EBalanceLifecycleState InCurrentState);

private:
	// Deferred Facing Synchronization
	void QueueDeferredCombatTargetFacingSync();
	void ResolveDeferredCombatTargetFacingSync();
	void CancelDeferredCombatTargetFacingSync();

private:
	// Facing Synchronization
	void SynchronizeCombatTargetFacing();
	void ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot);
	void ClearCombatTargetFacing();

private:
	// Facing Policy
	bool ShouldDeferCombatTargetFacingForReaction() const;
	bool IsCombatTargetFacingSuppressedByBalance() const;

private:
	// Component Reference Validation
	bool ValidateRequiredComponentReferences() const;
};
