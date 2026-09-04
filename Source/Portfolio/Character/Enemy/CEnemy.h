#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Type/CActionTypes.h"
#include "Type/CActionOrchestrationTypes.h"
#include "Type/CAITypes.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCharacterFeedbackTypes.h"
#include "Type/CCharacterSetupTypes.h"
#include "Type/CCombatResultTypes.h"
#include "Type/CCombatTargetTypes.h"
#include "Type/CEnemyAIConfigTypes.h"
#include "Type/CHealthTypes.h"
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

	struct FEnemyCombatActionAuthorityRuntime
	{
		FCombatTargetSnapshot ActiveTargetSnapshot;
		int32 ActiveParticipationRevision = 0;

		FCombatTargetSnapshot PendingTargetSnapshot;
		int32 PendingParticipationRevision = 0;
		uint32 PendingRequestSerial = 0;
		uint32 NextRequestSerial = 1;

		void ResetPending()
		{
			PendingTargetSnapshot = FCombatTargetSnapshot();
			PendingParticipationRevision = 0;
			PendingRequestSerial = 0;
		}

		void ResetAll()
		{
			ActiveTargetSnapshot = FCombatTargetSnapshot();
			ActiveParticipationRevision = 0;
			ResetPending();
		}

		uint32 AllocateRequestSerial()
		{
			if (NextRequestSerial == 0)
			{
				NextRequestSerial = 1;
			}

			return NextRequestSerial++;
		}
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
	// AI Policy Config
	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol", meta = (ShowOnlyInnerProperties))
	FEnemyPatrolConfig PatrolConfig;

	UPROPERTY(EditAnywhere, Category = "AI|Investigate", meta = (ShowOnlyInnerProperties))
	FEnemyInvestigateConfig InvestigateConfig;

	UPROPERTY(EditAnywhere, Category = "AI|Chase", meta = (ShowOnlyInnerProperties))
	FEnemyChaseConfig ChaseConfig;

	UPROPERTY(EditAnywhere, Category = "AI|Alert", meta = (ShowOnlyInnerProperties))
	FEnemyAlertConfig AlertConfig;

	UPROPERTY(EditAnywhere, Category = "AI|Engage", meta = (ShowOnlyInnerProperties))
	FEnemyEngageConfig EngageConfig;

private:
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UCWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere, Category = "State")
	class UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere, Category = "Resource")
	class UCHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, Category = "Resource")
	class UCBalanceComponent* BalanceComponent;

	UPROPERTY(VisibleAnywhere, Category = "Overlay")
	class UCObservableOverlayComponent* ObservableOverlayComponent;

	UPROPERTY(VisibleAnywhere, Category = "CombatTarget")
	class UCCombatTargetComponent* CombatTargetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Execution")
	class UCExecutionCollaborationComponent* ExecutionCollaborationComponent;

	UPROPERTY(VisibleAnywhere, Category = "CombatTarget")
	class UCEnemyCombatTargetFacingComponent* EnemyCombatTargetFacingComponent;

	UPROPERTY(VisibleAnywhere, Category = "CombatTarget")
	class UCEnemyCombatParticipationComponent* EnemyCombatParticipationComponent;

	UPROPERTY(VisibleAnywhere, Category = "CombatTarget")
	class UCEnemyHitReactiveComponent* EnemyHitReactiveComponent;

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
	// Runtime LOD State
	FRuntimeLODMeshState RuntimeLODMeshState;
	FRuntimeLODActorTickState RuntimeLODActorTickState;

private:
	// Combat Action Authority Runtime
	FEnemyCombatActionAuthorityRuntime CombatActionAuthorityRuntime;

private:
	// Death Lifecycle State
	UPROPERTY(EditDefaultsOnly, Category = "Death|Presentation", meta = (ClampMin = 0.0))
	float DeathPresentationFallbackDelay = 3.f;

private:
	FTimerHandle DeathEntryReactionStartFallbackTimerHandle;
	FTimerHandle DeathPresentationFallbackTimerHandle;
	FTimerHandle DeathFinalizeTimerHandle;

	bool bDeathLifecycleActive = false;
	bool bDeathPresentationRequested = false;
	bool bDeathFinalizationRequested = false;
	bool bDeathFinalized = false;
	bool bDeathPawnCollisionPolicyApplied = false;
	ECollisionResponse CachedPawnCollisionResponseBeforeDeath = ECR_Block;

	EDeathPresentationMode DeathPresentationMode = EDeathPresentationMode::Default;
	FExecutionSessionId ExpectedExecutionLethalDeathSessionId = FExecutionSessionId();

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
	// Engine Override
	void Tick(float DeltaTime) override;
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Component Query
	FORCEINLINE UCMovementComponent* GetMovementComp() const { return MovementComponent; }

	UFUNCTION(BlueprintPure, Category = "Component|Weapon")
	FORCEINLINE UCWeaponComponent* GetWeaponComp() const { return WeaponComponent; }

	FORCEINLINE UCStateComponent* GetStateComp() const { return StateComponent; }
	FORCEINLINE UCHealthComponent* GetHealthComp() const { return HealthComponent; }
	FORCEINLINE UCBalanceComponent* GetBalanceComp() const { return BalanceComponent; }
	FORCEINLINE UCObservableOverlayComponent* GetObservableOverlayComp() const { return ObservableOverlayComponent; }
	FORCEINLINE UCCombatTargetComponent* GetCombatTargetComp() const { return CombatTargetComponent; }
	FORCEINLINE UCExecutionCollaborationComponent* GetExecutionCollaborationComp() const { return ExecutionCollaborationComponent; }
	FORCEINLINE UCEnemyCombatTargetFacingComponent* GetEnemyCombatTargetFacingComp() const { return EnemyCombatTargetFacingComponent; }
	FORCEINLINE UCEnemyCombatParticipationComponent* GetEnemyCombatParticipationComp() const { return EnemyCombatParticipationComponent; }
	FORCEINLINE UCEnemyHitReactiveComponent* GetEnemyHitReactiveComp() const { return EnemyHitReactiveComponent; }
	FORCEINLINE UCCombatSignalSourceComponent* GetCombatSignalSourceComp() const { return CombatSignalSourceComponent; }
	FORCEINLINE UCCombatSignalTargetComponent* GetCombatSignalTargetComp() const { return CombatSignalTargetComponent; }
	FORCEINLINE UCActionOrchestratorComponent* GetActionOrchestratorComp() const { return ActionOrchestratorComponent; }
	FORCEINLINE UCReactionOrchestratorComponent* GetReactionOrchestratorComp() const { return ReactionOrchestratorComponent; }
	FORCEINLINE UCActionComponent* GetActionComp() const { return ActionComponent; }
	FORCEINLINE UCReactionComponent* GetReactionComp() const { return ReactionComponent; }
	FORCEINLINE UCHitFeedbackComponent* GetHitFeedbackComp() const { return HitFeedbackComponent; }
	FORCEINLINE UCActionFeedbackComponent* GetActionFeedbackComp() const { return ActionFeedbackComponent; }

	UFUNCTION(BlueprintPure, Category = "Component|Feedback")
	FORCEINLINE UCCharacterFeedbackComponent* GetCharacterFeedbackComp() const { return CharacterFeedbackComponent; }

	// Target Presentation Query
	FVector GetTargetMarkerWorldLocation() const;

	// AI Config Query
	FORCEINLINE const FEnemyPatrolConfig& GetPatrolConfig() const { return PatrolConfig; }
	FORCEINLINE const FEnemyInvestigateConfig& GetInvestigateConfig() const { return InvestigateConfig; }
	FORCEINLINE const FEnemyChaseConfig& GetChaseConfig() const { return ChaseConfig; }
	FORCEINLINE const FEnemyAlertConfig& GetAlertConfig() const { return AlertConfig; }
	FORCEINLINE const FEnemyEngageConfig& GetEngageConfig() const { return EngageConfig; }

	// Damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	// Combat Result
	void ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket) override;

public:
	// AI Movement Intent
	FActionRequestResult HandleAIWalk();
	FActionRequestResult HandleAIRun();
	FActionRequestResult HandleAISprint();

	FActionRequestResult HandleAIJump();
	FActionRequestResult HandleAIStopJump();

	// AI Equip Action Intent
	FActionRequestResult HandleAIEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent);

	// AI Combat Action Intent
	FActionRequestResult HandleAICombatAction(ECombatActionIntent InCombatActionIntent);

private:
	// Enemy Combat Action Authority Bridge
	// Action Event Observation
	UFUNCTION()
	void OnActionTypeChanged(class ACharacter* InOwnerCharacter, EActionType InPreviousActionType, EActionType InNewActionType);

	UFUNCTION()
	void OnActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, uint32 InActionRequestSerial, EActionEventType InActionEventType);

	// Authority Transition
	bool TryAcquireCombatActionAuthority(uint32 InActionRequestSerial);
	void ReleaseCombatActionAuthority();

	// Chain Action Routing
	void RequestChainCombatAction(EActionType InActionType, int32 InActionIndex);
	ECombatActionIntent ResolveChainCombatIntent(EActionType InActionType, int32 InActionIndex) const;

	// Classification
	bool IsCombatActionType(EActionType InActionType) const;

public:
	// Health / Death Command
	bool TryStartKill();

public:
	// -----------------------------------------------------------------------------
	// Death Lifecycle
	// -----------------------------------------------------------------------------
	// Death Lifecycle Query
	FORCEINLINE bool IsDeathLifecycleActive() const { return bDeathLifecycleActive; }
	FORCEINLINE EDeathPresentationMode GetDeathPresentationMode() const { return DeathPresentationMode; }
	FORCEINLINE const FExecutionSessionId& GetExpectedExecutionLethalDeathSessionId() const { return ExpectedExecutionLethalDeathSessionId; }
	FORCEINLINE bool IsDeathPresentationRequested() const { return bDeathPresentationRequested; }
	bool IsDeathPresentationFallbackPending() const;
	FORCEINLINE bool IsDeathFinalizationRequested() const { return bDeathFinalizationRequested; }
	FORCEINLINE bool IsDeathFinalized() const { return bDeathFinalized; }

private:
	// -----------------------------------------------------------------------------
	// Death Lifecycle Implementation
	// -----------------------------------------------------------------------------
	// Entry / State
	void HandleOwnerDeadStateChanged(EDeadState InPreviousDeadState, EDeadState InNewDeadState);
	void HandleExecutionLethalDeathEntryExpected(const FExecutionSessionId& InSessionId);
	void BeginDeathLifecycle();
	void AbortDeathLifecycle();

	// Keep world collision during death presentation while allowing characters to pass through the corpse.
	void ApplyDeathPawnCollisionPolicy();
	void RestoreDeathPawnCollisionPolicy();

	// Death Entry Reaction Contract
	EReactionType GetExpectedDeathEntryReactionType() const;
	bool IsExpectedDeathEntryReaction(const FReactionExecutionContext& InContext) const;

	// Death Entry Reaction Observation
	void HandleDeathEntryReactionLifecycleEvent(const FReactionExecutionLifecycleEvent& InEvent);

	// Death Entry Reaction Start Fallback
	void ValidateDeathEntryReactionStarted();

	// Death Presentation
	void BeginDeathPresentation(EDeathPresentationReason InReason);
	void ScheduleDeathPresentationFallback();

	void HandleDeathPresentationEvent(EDeathPresentationEventType InEventType);
	void HandleDeathPresentationFallbackExpired();

	// Death Finalization
	void RequestFinalizeDeath(EDeathFinalizeReason InReason);
	void FinalizeDeath();
	void CleanupDeathGameplayRuntime();
};
