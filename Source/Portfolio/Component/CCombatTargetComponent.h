#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCombatTargetTypes.h"
#include "CCombatTargetComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCombatTargetChanged, const FCombatTargetChange&);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCCombatTargetComponent();

private:
	// Runtime State
	TWeakObjectPtr<AActor> CurrentTarget;
	int32 CombatTargetRevision = 0;
	ECombatTargetChangeReason LastChangeReason = ECombatTargetChangeReason::None;

public:
	// Event
	FOnCombatTargetChanged OnCombatTargetChanged;

protected:
	// Lifecycle
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Target Command
	UFUNCTION(BlueprintCallable, Category = "CombatTarget")
	bool RequestSetCombatTarget(AActor* InTarget, ECombatTargetChangeReason InReason);

	UFUNCTION(BlueprintCallable, Category = "CombatTarget")
	bool RequestClearCombatTarget(ECombatTargetChangeReason InReason);

	bool RequestClearCombatTargetIfCurrent(AActor* InExpectedTarget, int32 InExpectedRevision, ECombatTargetChangeReason InReason);

public:
	// Target Query
	UFUNCTION(BlueprintPure, Category = "CombatTarget")
	bool HasCombatTarget() const;

	UFUNCTION(BlueprintPure, Category = "CombatTarget")
	AActor* GetCombatTargetActor() const;

	UFUNCTION(BlueprintPure, Category = "CombatTarget")
	FCombatTargetSnapshot GetCombatTargetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "CombatTarget")
	FORCEINLINE int32 GetCombatTargetRevision() const { return CombatTargetRevision; }

	UFUNCTION(BlueprintPure, Category = "CombatTarget")
	FORCEINLINE ECombatTargetChangeReason GetLastChangeReason() const { return LastChangeReason; }

private:
	// Validation
	bool ResolveStaleCombatTarget();

private:
	// Target Lifecycle
	void BindCombatTarget(AActor* InTarget);
	void UnbindCombatTarget(AActor* InTarget);
	void CleanupCombatTargetForOwnerEndPlay();

	UFUNCTION()
	void HandleCombatTargetEndPlay(AActor* InActor, EEndPlayReason::Type InEndPlayReason);

private:
	// Target State
	void BroadcastCombatTargetChanged(AActor* InPreviousTarget);
	bool CommitSetCombatTarget(AActor* InPreviousTarget, AActor* InCurrentTarget, ECombatTargetChangeReason InReason);
	bool CommitClearCombatTarget(AActor* InPreviousTarget, ECombatTargetChangeReason InReason);
};
