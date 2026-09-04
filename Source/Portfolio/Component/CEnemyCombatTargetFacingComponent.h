#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CEnemyCombatTargetFacingTypes.h"
#include "Type/CMovementTypes.h"
#include "CEnemyCombatTargetFacingComponent.generated.h"

class AAIController;
class UCCombatTargetComponent;
class UCBalanceComponent;
class UCHealthComponent;
class UCMovementComponent;
class UCReactionComponent;
enum class EBalanceLifecycleState : uint8;
enum class EDeadState : uint8;
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

	UPROPERTY(Transient)
	UCHealthComponent* HealthComponent_Injected = nullptr;

	// AI Controller Binding
	UPROPERTY(Transient)
	AAIController* AIController_Bound = nullptr;

	// Deferred Facing Synchronization Runtime
	bool bDeferredCombatTargetFacingSyncPending = false;
	bool bDeferredCombatTargetFacingSyncQueued = false;
	FTimerHandle DeferredCombatTargetFacingSyncTimerHandle;
	EEnemyCombatTargetFacingFocusDirective LastExpectedFocusDirective = EEnemyCombatTargetFacingFocusDirective::None;
	EEnemyCombatTargetFacingRotationDirective LastExpectedRotationDirective = EEnemyCombatTargetFacingRotationDirective::None;
	uint32 LastTransitionSequence = 0;
	float LastTransitionWorldTimeSeconds = 0.f;
	FString LastFacingEventName;
	FString LastFacingDecision;

public:
	// Component Reference
	void InitializeReferences(const FCharacterComponentReferences& InReferences);

	// AI Controller Binding
	void SetAIController(AAIController* InAIController);
	void ClearAIController();
	void ClearGameplayFocusFromExternal(AAIController* InAIController, const TCHAR* InSource);
	FEnemyCombatTargetFacingRuntimeSnapshot GetRuntimeSnapshot() const;

protected:
	// Lifecycle
	virtual void BeginPlay() override;
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
	void HandleDeadStateChanged(EDeadState InPreviousDeadState, EDeadState InCurrentDeadState);

private:
	// Deferred Facing Synchronization
	void QueueDeferredCombatTargetFacingSync();
	void ResolveDeferredCombatTargetFacingSync();
	void CancelDeferredCombatTargetFacingSync();
	void SynchronizeAIControllerBindingFromOwner(const TCHAR* InEvent);
	AAIController* ResolveOwnerAIController() const;

private:
	// Facing Synchronization
	void SynchronizeCombatTargetFacing(const TCHAR* InEvent);
	void ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot, const TCHAR* InEvent);
	void ClearCombatTargetFacing(const TCHAR* InEvent, const TCHAR* InDecision);
	void RecordCombatTargetFacingDecision(const TCHAR* InEvent, const TCHAR* InDecision, EMovementRotationMode InPreviousRotationMode, AActor* InPreviousGameplayFocusActor = nullptr);

private:
	// Facing Policy
	bool ShouldDeferCombatTargetFacingForReaction() const;
	bool ShouldSuppressCombatTargetFacing() const;

private:
	// Component Reference Validation
	bool ValidateRequiredComponentReferences() const;
};
