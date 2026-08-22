#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CTargetingTypes.h"
#include "CTargetLockAssistComponent.generated.h"

class ACEnemy;
class ACPlayer;
class APlayerController;
class UCCombatTargetComponent;
struct FCombatTargetChange;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCTargetLockAssistComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCTargetLockAssistComponent();

private:
	// Tuning
	UPROPERTY(EditAnywhere, Category = "Targeting|LockAssist")
	FTargetLockAssistTuning TargetLockAssistTuning;

private:
	// Component Reference
	UPROPERTY(Transient)
	APlayerController* OwnerPlayerController_Injected = nullptr;

	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;

private:
	// Runtime State
	TWeakObjectPtr<ACPlayer> ControlledPlayer;

public:
	// Component Reference
	void InitializeReferences(APlayerController* InOwnerPlayerController);
	void SetCombatTargetComponent(UCCombatTargetComponent* InCombatTargetComponent);
	void SetControlledPlayer(ACPlayer* InControlledPlayer);
	void ClearControlledPlayer();

protected:
	// Lifecycle
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Query
	bool IsTargetLockActive() const;
	bool ShouldSuppressLookInput() const;

private:
	// Validation
	bool ValidateRequiredReferences() const;

private:
	// Target State
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);
	void ApplyCurrentTargetPolicy();
	void RestoreControlledPlayerPolicy();

private:
	// Camera Tracking
	void UpdateCameraTracking(float DeltaTime);
	float ResolveDesiredLockPitch(float InRawTargetPitch, float InHorizontalDistance) const;
};
