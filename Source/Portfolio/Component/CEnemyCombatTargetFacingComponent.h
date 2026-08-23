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
	UCEnemyCombatTargetFacingComponent();

private:
	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;

	UPROPERTY(Transient)
	UCMovementComponent* MovementComponent_Injected = nullptr;

	UPROPERTY(Transient)
	UCReactionComponent* ReactionComponent_Injected = nullptr;

	UPROPERTY(Transient)
	UCBalanceComponent* BalanceComponent_Injected = nullptr;

	UPROPERTY(Transient)
	AAIController* AIController_Injected = nullptr;

	bool bCombatTargetFacingSyncPending = false;
	bool bCombatTargetFacingSyncQueued = false;
	FTimerHandle CombatTargetFacingSyncTimerHandle;

public:
	void InitializeReferences(const FCharacterComponentReferences& InReferences);
	void SetAIController(AAIController* InAIController);
	void ClearAIController();

protected:
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

private:
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);
	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);
	void HandleBalanceLifecycleStateChanged(EBalanceLifecycleState InPreviousState, EBalanceLifecycleState InCurrentState);
	void SynchronizeCombatTargetFacing();
	void QueueCombatTargetFacingSync();
	void ResolveQueuedCombatTargetFacingSync();
	void CancelQueuedCombatTargetFacingSync();

private:
	bool ShouldDeferCombatTargetFacing() const;
	bool IsCombatTargetFacingSuppressed() const;
	void ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot);
	void ClearCombatTargetFacing();
};
