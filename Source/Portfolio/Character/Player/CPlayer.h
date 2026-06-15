#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/TargetContextProvider.h"
#include "Type/CActionOrchestrationStructure.h"
#include "CPlayer.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayer : public ACharacter, public ITargetContextProvider
{
	GENERATED_BODY()

public:
	ACPlayer();

private:
	UPROPERTY(EditAnywhere, Category = "Priority")
	int Priority = INT_MAX;

private:
	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Orchestrator")
	class UCActionOrchestratorComponent* ActionOrchestratorComponent;

	UPROPERTY(VisibleAnywhere, Category = "Orchestrator")
	class UCReactionOrchestratorComponent* ReactionOrchestratorComponent;

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

	UPROPERTY(VisibleAnywhere, Category = "HandlingDamage")
	class UCApplyDamageComponent* ApplyDamageComponent;

	UPROPERTY(VisibleAnywhere, Category = "HandlingDamage")
	class UCTakeDamageComponent* TakeDamageComponent;

	UPROPERTY(VisibleAnywhere, Category = "Execution")
	class UCActionComponent* ActionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Execution")
	class UCReactionComponent* ReactionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	class UCDamageFeedbackComponent* DamageFeedbackComponent;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	class UCActionFeedbackComponent* ActionFeedbackComponent;

	UPROPERTY(VisibleAnywhere, Category = "Feedback")
	class UCReactionFeedbackComponent* ReactionFeedbackComponent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE UCActionOrchestratorComponent* GetActionOrchestratorComp() const { return ActionOrchestratorComponent; }
	FORCEINLINE UCReactionOrchestratorComponent* GetReactionOrchestratorComp() const { return ReactionOrchestratorComponent; }
	
	FORCEINLINE UCMovementComponent* GetMovementComp() const { return MovementComponent; }
	FORCEINLINE UCWeaponComponent* GetWeaponComp() const { return WeaponComponent; }
	FORCEINLINE UCStateComponent* GetStateComp() const { return StateComponent; }
	FORCEINLINE UCHealthComponent* GetHealthComp() const { return HealthComponent; }
	FORCEINLINE UCDefenseComponent* GetDefenseComp() const { return DefenseComponent; }
	FORCEINLINE UCApplyDamageComponent* GetApplyDamageComp() const { return ApplyDamageComponent; }
	FORCEINLINE UCTakeDamageComponent* GetTakeDamageComp() const { return TakeDamageComponent; }
	FORCEINLINE UCActionComponent* GetActionComp() const { return ActionComponent; }
	FORCEINLINE UCReactionComponent* GetReactionComp() const { return ReactionComponent; }
	FORCEINLINE UCActionFeedbackComponent* GetActionFeedbackComp() const { return ActionFeedbackComponent; }
	FORCEINLINE UCDamageFeedbackComponent* GetDamageFeedbackComp() const { return DamageFeedbackComponent; }

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

public:
	// Interface API
	int GetTargetPriority() const override { return Priority; }

public:
	FActionRequestResult HandleMove(const FVector2D& InAxis2D);

public:
	FActionRequestResult HandleWalk();
	FActionRequestResult HandleRun();
	FActionRequestResult HandleSprint();

	FActionRequestResult HandleJump();
	FActionRequestResult HandleStopJump();

	FActionRequestResult HandleEquipmentAction(EEquipmentActionIntent InEquipmentActionIntent);
	FActionRequestResult HandleCombatAction(ECombatActionIntent InCombatActionIntent, EActionIntentEvent InIntentEvent = EActionIntentEvent::Started);
};
