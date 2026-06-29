#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Type/CAIStructure.h"
#include "Type/CActionOrchestrationStructure.h"
#include "Type/CCharacterComponentReferenceStructure.h"
#include "Type/CWeaponStructure.h"
#include "Interface/CombatResultReceiver.h"
#include "CEnemy.generated.h"

UCLASS()
class PORTFOLIO_API ACEnemy : public ACharacter, public ICombatResultReceiver
{
	GENERATED_BODY()

public:
	ACEnemy();

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

private:
	UPROPERTY(EditAnywhere, Category = "CombatResult|Parry")
	int32 ParryStaggerThreshold = 3;

	UPROPERTY(VisibleInstanceOnly, Category = "CombatResult|Parry")
	int32 ParryResultCount = 0;

protected:
	// Lifecycle
	void PostInitializeComponents() override;
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Component Reference
	void RecoverReferences();
	void BuildReferences(FCharacterComponentReferences& OutReferences);
	void InjectReferences(const FCharacterComponentReferences& InReferences);

public:
	void Tick(float DeltaTime) override;

public:
	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
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

public:
	FORCEINLINE bool GetbUsePatrol() const { return bUsePatrol; }
	FORCEINLINE ACPatrolPath* GetPatrolPath() const { return PatrolPath; }
	FORCEINLINE EPatrolMode GetPatrolMode() const { return PatrolMode; }

public:
	FORCEINLINE bool GetbUseInvestigate() const { return bUseInvestigate; }
	FORCEINLINE float GetInvestigateDuration() const { return InvestigateDuration; }
	FORCEINLINE int32 GetInvestigateMaxIndex() const { return InvestigateMaxIndex; }

public:
	FORCEINLINE float GetChaseOffsetRange() const { return ChaseOffsetRange; }
	FORCEINLINE float GetChaseEnterBuffer() const { return ChaseEnterBuffer; }
	FORCEINLINE float GetChaseExitBuffer() const { return ChaseExitBuffer; }

public:
	FORCEINLINE bool GetbUseAlertStep() const { return bUseAlertStep; }
	FORCEINLINE float GetStepSideDistance() const { return StepSideDistance; }
	FORCEINLINE float GetStepForwardDistance() const { return StepForwardDistance; }

public:
	FORCEINLINE float GetEngageOffsetRange() const { return EngageOffsetRange; }
	FORCEINLINE float GetEngageEnterBuffer() const { return EngageEnterBuffer; }
	FORCEINLINE float GetEngageExitBuffer() const { return EngageExitBuffer; }

public:
	FORCEINLINE float GetCombatActionCooldown() const { return CombatActionCooldown; }

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

public:
	void ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket) override;

private:
	void HandleParryCombatResult(const FCombatResultPacket& InCombatResultPacket);
	bool TryRequestParryStaggerReaction(const FCombatResultPacket& InCombatResultPacket);

public:
	FActionRequestResult HandleAIWalk();
	FActionRequestResult HandleAIRun();
	FActionRequestResult HandleAISprint();

	FActionRequestResult HandleAIJump();
	FActionRequestResult HandleAIStopJump();

	FActionRequestResult HandleAIEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent);
	FActionRequestResult HandleAICombatAction(ECombatActionIntent InCombatActionIntent);

public:
	bool TryStartKill();
	bool TryStartRevive(float InReviveHP);

private:
	bool IsCombatActionType(EActionType InActionType) const;

private:
	UFUNCTION()
	void OnActionTypeChanged(class ACharacter* InOwnerCharacter, EActionType InPreviousActionType, EActionType InNewActionType);

private:
	UFUNCTION()
	void OnActionEvent(ACharacter* InOwnerCharacter, EActionType InActionType, int32 InActionIndex, EActionEventType InActionEventType);

private:
	// ActionEvent API (Event -> Intent -> Handle)
	void RequestChainCombatAction(EActionType InActionType, int32 InActionIndex);
	ECombatActionIntent ResolveChainCombatIntent(EActionType InActionType, int32 InActionIndex) const;
};
