#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CEnemyCombatTargetFacingComponent.generated.h"

class AAIController;
class UCCombatTargetComponent;
class UCMovementComponent;
struct FCharacterComponentReferences;
struct FCombatTargetChange;
struct FCombatTargetSnapshot;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCEnemyCombatTargetFacingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCEnemyCombatTargetFacingComponent();

private:
	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;

	UPROPERTY(Transient)
	UCMovementComponent* MovementComponent_Injected = nullptr;

	UPROPERTY(Transient)
	AAIController* AIController_Injected = nullptr;

public:
	void InitializeReferences(const FCharacterComponentReferences& InReferences);
	void SetAIController(AAIController* InAIController);
	void ClearAIController();

protected:
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

private:
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);
	void SynchronizeCombatTargetFacing();

private:
	void ApplyCombatTargetFacing(const FCombatTargetSnapshot& InSnapshot);
	void ClearCombatTargetFacing();
};
