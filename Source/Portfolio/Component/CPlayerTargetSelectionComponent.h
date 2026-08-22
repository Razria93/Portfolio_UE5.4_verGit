#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CTargetingTypes.h"
#include "CPlayerTargetSelectionComponent.generated.h"

class AActor;
class ACEnemy;
class UCCombatTargetComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCPlayerTargetSelectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPlayerTargetSelectionComponent();

private:
	// Tuning
	UPROPERTY(EditAnywhere, Category = "Targeting")
	FTargetingTuning TargetingTuning;

private:
	// Component Reference
	UPROPERTY(Transient)
	class APlayerController* OwnerPlayerController_Injected = nullptr;

private:
	// Runtime State
	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;

	float CurrentTargetMaintenanceElapsedTime = 0.f;

public:
	// Component Reference
	void InitializeReferences(class APlayerController* InOwnerPlayerController);
	void SetCombatTargetComponent(UCCombatTargetComponent* InCombatTargetComponent);

private:
	// Validation
	bool ValidateRequiredReferences() const;

protected:
	// Lifecycle
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Player Target Selection Command
	void ToggleCombatTargetSelection();
	bool SelectBestTarget();
	bool SelectAdjacentTarget(ETargetSwitchDirection InDirection);
	void ClearCombatTarget();

public:
	// Debug Query
	bool BuildSelectionDebugSnapshot(FTargetingEvaluation& OutEvaluation) const;

private:
	// Current Target Maintenance
	void MaintainCurrentTarget();

private:
	// Target Evaluation
	bool BuildTargetEvaluation(const ACEnemy* InTarget, FTargetingEvaluation& OutEvaluation) const;

private:
	// Target Criteria
	bool CanSelectTarget(const ACEnemy* InTarget, const FTargetingEvaluation& InEvaluation) const;
	bool CanRetainCombatTarget(const ACEnemy* InTarget, const FTargetingEvaluation& InEvaluation) const;

private:
	// Candidate Selection
	bool TryCalculateTargetScore(const ACEnemy* InTarget, float& OutScore) const;
	bool ProjectTargetToViewport(const ACEnemy* InTarget, FVector2D& OutScreenPosition) const;

};
