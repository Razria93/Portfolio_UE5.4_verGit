#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Type/CAITypes.h"
#include "Type/CActionOrchestrationTypes.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCharacterSetupTypes.h"
#include "Type/CActionTypes.h"
#include "Type/CHealthTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CCharacterFeedbackTypes.h"
#include "Type/CReactionOrchestrationTypes.h"
#include "Interface/CombatResultReceiver.h"
#include "CEnemy.generated.h"

UCLASS()
class PORTFOLIO_API ACEnemy : public ACharacter, public ICombatResultReceiver
{
	GENERATED_BODY()

public:
	ACEnemy();

private:
	struct FRuntimeLODMeshState
	{
		int32 AppliedMode = INDEX_NONE;
		uint8 OriginalVisibilityBasedAnimTickOption = 0;
		bool bOriginalStateCached = false;
	};

	struct FRuntimeLODActorTickState
	{
		int32 AppliedMode = INDEX_NONE;
		bool bOriginalActorTickEnabled = true;
		bool bOriginalStateCached = false;
	};

private:
	UPROPERTY(EditDefaultsOnly, Category = "CharacterSetup")
	FCharacterCapsuleSetup CapsuleSetup;

	UPROPERTY(EditDefaultsOnly, Category = "CharacterSetup")
	FCharacterMeshSetup MeshSetup;

	UPROPERTY(EditDefaultsOnly, Category = "CharacterSetup")
	FCharacterMovementSetup MovementSetup;

private:
	// Target Presentation
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Presentation")
	FName TargetMarkerSocketName = TEXT("TargetMarker");

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|Presentation")
	FVector TargetMarkerFallbackOffset = FVector::ZeroVector;

private:
	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	bool bUsePatrol;

	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	class ACPatrolPath* PatrolPath;

	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	EPatrolMode PatrolMode = EPatrolMode::None;

private:
	UPROPERTY(EditAnywhere, Category = "AI|Investigate")
	bool bUseInvestigate;

	UPROPERTY(EditAnywhere, Category = "AI|Investigate")
	float InvestigateDuration;

	UPROPERTY(EditAnywhere, Category = "AI|Investigate")
	int32 InvestigateMaxIndex;

private:
	UPROPERTY(EditAnywhere, Category = "AI|Chase")
	float ChaseOffsetRange;

	UPROPERTY(EditAnywhere, Category = "AI|Chase")
	float ChaseEnterBuffer;

	UPROPERTY(EditAnywhere, Category = "AI|Chase")
	float ChaseExitBuffer;

private:
	UPROPERTY(EditAnywhere, Category = "AI|Alert")
	bool bUseAlertStep;

	UPROPERTY(EditAnywhere, Category = "AI|Alert")
	float StepSideDistance;

	UPROPERTY(EditAnywhere, Category = "AI|Alert")
	float StepForwardDistance;

private:
	UPROPERTY(EditAnywhere, Category = "AI|Engage")
	float EngageOffsetRange;

	UPROPERTY(EditAnywhere, Category = "AI|Engage")
	float EngageEnterBuffer;

	UPROPERTY(EditAnywhere, Category = "AI|Engage")
	float EngageExitBuffer;

	UPROPERTY(EditAnywhere, Category = "AI|Engage")
	float CombatActionCooldown;

private:
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UCWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere, Category = "State")
	class UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere, Category = "Resource")
	class UCHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, Category = "Overlay")
	class UCObservableOverlayComponent* ObservableOverlayComponent;

	UPROPERTY(VisibleAnywhere, Category = "CombatSignal")
	class UCCombatSignalSourceComponent* CombatSignalSourceComponent;

	UPROPERTY(VisibleAnywhere, Category = "CombatSignal")
	class UCCombatSignalTargetComponent* CombatSignalTargetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Orchestrator")
	class UCActionOrchestratorComponent* ActionOrchestratorComponent;

	UPROPERTY(VisibleAnywhere, Category = "Orchestrator")
	class UCReactionOrchestratorComponent* ReactionOrchestratorComponent;

	UPROPERTY(VisibleAnywhere, Category = "Execution")
	class UCActionComponent* ActionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Execution")
	class UCReactionComponent* ReactionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	class UCHitFeedbackComponent* HitFeedbackComponent;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	class UCActionFeedbackComponent* ActionFeedbackComponent;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	class UCReactionFeedbackComponent* ReactionFeedbackComponent;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	class UCCharacterFeedbackComponent* CharacterFeedbackComponent;

private:
	UPROPERTY(EditAnywhere, Category = "CombatResult|Parry", meta = (ClampMin = 1))
	int32 ParryStaggerThreshold = 3;

	UPROPERTY(VisibleInstanceOnly, Category = "CombatResult|Parry")
	int32 ParryResultCount = 0;

private:
	FRuntimeLODMeshState RuntimeLODMeshState;
	FRuntimeLODActorTickState RuntimeLODActorTickState;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Death|Watchdog", meta = (ClampMin = 0.0))
	float DeathPresentationWatchdogMinimumDuration = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Death|Watchdog", meta = (ClampMin = 0.0))
	float DeathPresentationWatchdogSafetyMargin = 0.5f;

private:
	FTimerHandle DeadReactionStartFallbackTimerHandle;
	FTimerHandle DeathPresentationWatchdogTimerHandle;
	FTimerHandle DeathFinalizeTimerHandle;

	bool bDeathLifecycleActive = false;
	bool bDeathPresentationStarted = false;
	bool bDeathFinalizationRequested = false;
	bool bDeathFinalized = false;

protected:
	// Lifecycle
	void OnConstruction(const FTransform& Transform) override;
	void PostInitializeComponents() override;
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Setup
	void ApplyCharacterSetup();

private:
	// Component Reference
	void RecoverReferences();
	void BuildReferences(FCharacterComponentReferences& OutReferences);
	void InjectReferences(const FCharacterComponentReferences& InReferences);

private:
	// Runtime LOD
	void UpdateRuntimeLODMeshMode();
	void UpdateRuntimeLODActorTickMode();
	void CacheRuntimeLODActorTickOriginalState();
	void ApplyRuntimeLODActorTickMode(int32 InActorTickMode);
	void ApplyRuntimeLODActorTickDefault();
	void ApplyRuntimeLODActorTickDisabled();
	void RestoreRuntimeLODActorTick();
	void DisableRuntimeLODActorTick();

public:
	// Tick
	void Tick(float DeltaTime) override;

public:
	// Input
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Component Query
	FORCEINLINE UCMovementComponent* GetMovementComp() const { return MovementComponent; }
	FORCEINLINE UCWeaponComponent* GetWeaponComp() const { return WeaponComponent; }
	FORCEINLINE UCStateComponent* GetStateComp() const { return StateComponent; }
	FORCEINLINE UCHealthComponent* GetHealthComp() const { return HealthComponent; }
	FORCEINLINE UCObservableOverlayComponent* GetObservableOverlayComp() const { return ObservableOverlayComponent; }
	FORCEINLINE UCCombatSignalSourceComponent* GetCombatSignalSourceComp() const { return CombatSignalSourceComponent; }
	FORCEINLINE UCCombatSignalTargetComponent* GetCombatSignalTargetComp() const { return CombatSignalTargetComponent; }
	FORCEINLINE UCActionOrchestratorComponent* GetActionOrchestratorComp() const { return ActionOrchestratorComponent; }
	FORCEINLINE UCReactionOrchestratorComponent* GetReactionOrchestratorComp() const { return ReactionOrchestratorComponent; }
	FORCEINLINE UCActionComponent* GetActionComp() const { return ActionComponent; }
	FORCEINLINE UCReactionComponent* GetReactionComp() const { return ReactionComponent; }
	FORCEINLINE UCHitFeedbackComponent* GetHitFeedbackComp() const { return HitFeedbackComponent; }
	FORCEINLINE UCActionFeedbackComponent* GetActionFeedbackComp() const { return ActionFeedbackComponent; }
	FORCEINLINE UCCharacterFeedbackComponent* GetCharacterFeedbackComp() const { return CharacterFeedbackComponent; }
	FORCEINLINE int32 GetParryResultCount() const { return ParryResultCount; }
	FORCEINLINE int32 GetParryStaggerThreshold() const { return ParryStaggerThreshold; }

public:
	// Target Presentation Query
	FVector GetTargetMarkerWorldLocation() const;

public:
	// AI Config Query
	FORCEINLINE bool ShouldUsePatrol() const { return bUsePatrol; }
	FORCEINLINE ACPatrolPath* GetPatrolPath() const { return PatrolPath; }
	FORCEINLINE EPatrolMode GetPatrolMode() const { return PatrolMode; }

public:
	FORCEINLINE bool ShouldUseInvestigate() const { return bUseInvestigate; }
	FORCEINLINE float GetInvestigateDuration() const { return InvestigateDuration; }
	FORCEINLINE int32 GetInvestigateMaxIndex() const { return InvestigateMaxIndex; }

public:
	FORCEINLINE float GetChaseOffsetRange() const { return ChaseOffsetRange; }
	FORCEINLINE float GetChaseEnterBuffer() const { return ChaseEnterBuffer; }
	FORCEINLINE float GetChaseExitBuffer() const { return ChaseExitBuffer; }

public:
	FORCEINLINE bool ShouldUseAlertStep() const { return bUseAlertStep; }
	FORCEINLINE float GetStepSideDistance() const { return StepSideDistance; }
	FORCEINLINE float GetStepForwardDistance() const { return StepForwardDistance; }

public:
	FORCEINLINE float GetEngageOffsetRange() const { return EngageOffsetRange; }
	FORCEINLINE float GetEngageEnterBuffer() const { return EngageEnterBuffer; }
	FORCEINLINE float GetEngageExitBuffer() const { return EngageExitBuffer; }

public:
	FORCEINLINE float GetCombatActionCooldown() const { return CombatActionCooldown; }

public:
	// Damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

public:
	// Combat Result
	void ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket) override;

private:
	void HandleParryCombatResult(const FCombatResultPacket& InCombatResultPacket);
	bool TryRequestParryStaggerReaction(const FCombatResultPacket& InCombatResultPacket);

public:
	// AI Movement Intent
	FActionRequestResult HandleAIWalk();
	FActionRequestResult HandleAIRun();
	FActionRequestResult HandleAISprint();

	FActionRequestResult HandleAIJump();
	FActionRequestResult HandleAIStopJump();

	// AI Action Intent
	FActionRequestResult HandleAIEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent);
	FActionRequestResult HandleAICombatAction(ECombatActionIntent InCombatActionIntent);

public:
	// Runtime State
	bool TryStartKill();

private:
	// Death Lifecycle Entry / State
	void HandleOwnerDeadStateChanged(EDeadState InPreviousDeadState, EDeadState InNewDeadState);
	void BeginDeathLifecycle();
	void AbortDeathLifecycle();

	// Death Reaction Observation / Fallback
	void HandleReactionExecutionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);
	void ValidateDeadReactionStarted();

	// Death Presentation
	void BeginDeathPresentation(EDeathPresentationReason InReason);
	void ScheduleDeathPresentationWatchdog(float InExpectedDuration);

	void HandleDeathPresentationFinished();
	void HandleDeathPresentationWatchdogExpired();

	// Death Finalization
	void RequestFinalizeDeath(EDeathFinalizeReason InReason);
	void FinalizeDeath();
	void CleanupDeathGameplayRuntime();

private:
	// Combat Action Query
	bool IsCombatActionType(EActionType InActionType) const;

private:
	UFUNCTION()
	void OnActionTypeChanged(class ACharacter* InOwnerCharacter, EActionType InPreviousActionType, EActionType InNewActionType);

private:
	UFUNCTION()
	void OnActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, EActionEventType InActionEventType);

private:
	// Action Event Routing
	void RequestChainCombatAction(EActionType InActionType, int32 InActionIndex);
	ECombatActionIntent ResolveChainCombatIntent(EActionType InActionType, int32 InActionIndex) const;
};
