#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/TargetContextProvider.h"
#include "Interface/CombatResultReceiver.h"
#include "Type/CActionOrchestrationTypes.h"
#include "Type/CCharacterComponentReferenceTypes.h"
#include "Type/CCharacterSetupTypes.h"
#include "CPlayer.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayer : public ACharacter, public ITargetContextProvider, public ICombatResultReceiver
{
	GENERATED_BODY()

public:
	ACPlayer();

private:
	UPROPERTY(EditAnywhere, Category = "Priority")
	int Priority = INT_MAX;

private:
	UPROPERTY(EditDefaultsOnly, Category = "CharacterSetup")
	FCharacterCapsuleSetup CapsuleSetup;

	UPROPERTY(EditDefaultsOnly, Category = "CharacterSetup")
	FCharacterMeshSetup MeshSetup;

	UPROPERTY(EditDefaultsOnly, Category = "CharacterSetup")
	FCharacterMovementSetup MovementSetup;

	UPROPERTY(EditDefaultsOnly, Category = "CharacterSetup")
	FPlayerCameraSetup CameraSetup;

private:
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	class UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	class UCWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere, Category = "State")
	class UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere, Category = "Resource")
	class UCHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, Category = "Defense")
	class UCDefenseComponent* DefenseComponent;

	UPROPERTY(VisibleAnywhere, Category = "Overlay")
	class UCObservableOverlayComponent* ObservableOverlayComponent;

	UPROPERTY(VisibleAnywhere, Category = "CombatTarget")
	class UCCombatTargetComponent* CombatTargetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Execution")
	class UCExecutionCollaborationComponent* ExecutionCollaborationComponent;

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

protected:
	// Lifecycle
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Setup
	void ApplyCharacterSetup();

private:
	// Component Reference
	void RecoverReferences();
	void BuildReferences(FCharacterComponentReferences& OutReferences);
	void InjectReferences(const FCharacterComponentReferences& InReferences);

public:
	// Input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Query
	FORCEINLINE UCMovementComponent* GetMovementComp() const { return MovementComponent; }
	FORCEINLINE UCWeaponComponent* GetWeaponComp() const { return WeaponComponent; }
	FORCEINLINE UCStateComponent* GetStateComp() const { return StateComponent; }
	FORCEINLINE UCHealthComponent* GetHealthComp() const { return HealthComponent; }
	FORCEINLINE UCDefenseComponent* GetDefenseComp() const { return DefenseComponent; }
	FORCEINLINE UCObservableOverlayComponent* GetObservableOverlayComp() const { return ObservableOverlayComponent; }
	FORCEINLINE UCCombatTargetComponent* GetCombatTargetComp() const { return CombatTargetComponent; }
	FORCEINLINE UCExecutionCollaborationComponent* GetExecutionCollaborationComp() const { return ExecutionCollaborationComponent; }
	FORCEINLINE UCCombatSignalSourceComponent* GetCombatSignalSourceComp() const { return CombatSignalSourceComponent; }
	FORCEINLINE UCCombatSignalTargetComponent* GetCombatSignalTargetComp() const { return CombatSignalTargetComponent; }
	FORCEINLINE UCActionOrchestratorComponent* GetActionOrchestratorComp() const { return ActionOrchestratorComponent; }
	FORCEINLINE UCReactionOrchestratorComponent* GetReactionOrchestratorComp() const { return ReactionOrchestratorComponent; }
	FORCEINLINE UCActionComponent* GetActionComp() const { return ActionComponent; }
	FORCEINLINE UCReactionComponent* GetReactionComp() const { return ReactionComponent; }
	FORCEINLINE UCHitFeedbackComponent* GetHitFeedbackComp() const { return HitFeedbackComponent; }
	FORCEINLINE UCActionFeedbackComponent* GetActionFeedbackComp() const { return ActionFeedbackComponent; }

public:
	// Damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

public:
	// Combat Result
	void ReceiveCombatResultPacket(const FCombatResultPacket& InCombatResultPacket) override;

public:
	// Interface
	int GetTargetPriority() const override { return Priority; }

public:
	// Movement Intent
	FActionRequestResult HandleMove(const FVector2D& InAxis2D);
	FActionRequestResult HandleLocomotionGaitInput(bool bWalkInputHeld, bool bSprintInputHeld);

public:
	// Action Intent
	FActionRequestResult HandleWalk();
	FActionRequestResult HandleRun();
	FActionRequestResult HandleSprint();

	FActionRequestResult HandleJump();
	FActionRequestResult HandleStopJump();

	FActionRequestResult HandleEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent);
	FActionRequestResult HandleCombatAction(ECombatActionIntent InCombatActionIntent, EActionIntentEvent InIntentEvent = EActionIntentEvent::Started);
	bool RequestExecutionForCurrentTarget();
};
