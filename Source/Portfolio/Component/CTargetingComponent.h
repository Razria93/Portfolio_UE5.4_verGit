#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CTargetingTypes.h"
#include "CTargetingComponent.generated.h"

class AActor;
class ACEnemy;
class UCCombatTargetComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCTargetingComponent();

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

	float ValidationElapsedTime = 0.f;

public:
	// Component Reference
	void InitializeReferences(class APlayerController* InOwnerPlayerController);
	void SetCombatTargetComponent(UCCombatTargetComponent* InCombatTargetComponent);

protected:
	// Lifecycle
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Target Command
	void ToggleTargetLock();
	bool AcquireBestTarget();
	bool SwitchTarget(ETargetSwitchDirection InDirection);
	void ClearTarget();

public:
	// Debug Query
	bool BuildDebugSnapshot(FTargetingDebugSnapshot& OutSnapshot) const;

private:
	// Validation
	bool ValidateRequiredReferences() const;
	bool IsTargetValid(const ACEnemy* InTarget, bool bRequireViewCone) const;
	void ValidateCurrentTarget();

private:
	// Target Evaluation
	bool BuildTargetEvaluation(const ACEnemy* InTarget, FTargetingDebugSnapshot& OutEvaluation) const;
	bool IsTargetEvaluationValid(const ACEnemy* InTarget, const FTargetingDebugSnapshot& InEvaluation, bool bRequireViewCone) const;

private:
	// Candidate Selection
	bool TryScoreTarget(const ACEnemy* InTarget, float& OutScore) const;
	bool ProjectTargetToViewport(const ACEnemy* InTarget, FVector2D& OutScreenPosition) const;

};
