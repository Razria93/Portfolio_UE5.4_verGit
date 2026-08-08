#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CTargetingTypes.h"
#include "CTargetingComponent.generated.h"

class ACEnemy;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTargetChanged, ACEnemy* /* PreviousTarget */, ACEnemy* /* NewTarget */);

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

	// Component Reference
	UPROPERTY(Transient)
	class APlayerController* OwnerPlayerController_Injected = nullptr;

	// Runtime State
	TWeakObjectPtr<ACEnemy> CurrentTarget;
	float ValidationElapsedTime = 0.f;

public:
	// Event
	FOnTargetChanged OnTargetChanged;

	// Component Reference
	void InitializeReferences(class APlayerController* InOwnerPlayerController);

protected:
	// Lifecycle
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Target Command
	void ToggleTargetLock();
	bool AcquireBestTarget();
	void ClearTarget();

	// Target Query
	bool HasTarget() const;
	ACEnemy* GetCurrentTarget() const;
	bool BuildDebugSnapshot(FTargetingDebugSnapshot& OutSnapshot) const;

private:
	// Validation
	bool ValidateRequiredReferences() const;
	bool IsTargetValid(const ACEnemy* InTarget, bool bRequireViewCone) const;
	void ValidateCurrentTarget();

	// Candidate Selection
	bool TryScoreTarget(const ACEnemy* InTarget, float& OutScore) const;

	// Target State
	void SetCurrentTarget(ACEnemy* InNewTarget);

};
