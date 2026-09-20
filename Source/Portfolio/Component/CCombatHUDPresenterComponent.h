#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CCombatHUDTypes.h"
#include "CCombatHUDPresenterComponent.generated.h"

class UCHealthComponent;
class UCBalanceComponent;
class UCCombatTargetComponent;
class UCCombatHUDWidget;
class ACPlayer;
class UCDefenseComponent;
class UCActionComponent;
class UCActionOrchestratorComponent;
class UCExecutionCollaborationComponent;
class UCCombatSignalTargetComponent;
class UCTargetLockAssistComponent;
struct FCombatTargetChange;
struct FCombatSignalTargetPacket;
enum class EDeadState : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatHUDPresenterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Construction
	UCCombatHUDPresenterComponent();

private:
	// Presentation Config
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD")
	TSubclassOf<UCCombatHUDWidget> WidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Combat HUD")
	int32 WidgetZOrder = 5;

private:
	// Controller
	TWeakObjectPtr<UCTargetLockAssistComponent> LockAssist;

	// Player References
	// - Character / Targeting
	TWeakObjectPtr<ACPlayer> Player;
	TWeakObjectPtr<UCCombatTargetComponent> PlayerCombatTarget;

	// - Health / Defense
	TWeakObjectPtr<UCHealthComponent> PlayerHealth;
	TWeakObjectPtr<UCDefenseComponent> Defense;

	// - Action / Execution
	TWeakObjectPtr<UCActionComponent> Actions;
	TWeakObjectPtr<UCActionOrchestratorComponent> ActionOrchestrator;
	TWeakObjectPtr<UCExecutionCollaborationComponent> Execution;

	// - Combat Result
	TWeakObjectPtr<UCCombatSignalTargetComponent> PlayerCombatSignals;

	// Target References
	TWeakObjectPtr<AActor> Target;
	TWeakObjectPtr<UCHealthComponent> TargetHealth;
	TWeakObjectPtr<UCBalanceComponent> TargetBalance;

private:
	// Runtime Widget
	UPROPERTY(Transient)
	TObjectPtr<UCCombatHUDWidget> Widget;

	// Presentation Runtime
	FCombatHUDViewData ViewData;
	double ParryHighlightUntil = 0.0;
	uint64 LastParryResultSerial = 0;

	// Lifecycle Runtime
	bool bEndingPlay = false;

public:
	// Component Reference
	void SetControlledPlayer(ACPlayer* InPlayer);

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Query
	const FCombatHUDViewData& GetViewData() const { return ViewData; }

private:
	// Player Reference Cache
	void CachePlayerReferences(ACPlayer* InPlayer);

private:
	// Player Event Binding
	void BindPlayerEvents();
	void UnbindPlayerEvents();

private:
	// Target Binding
	void RefreshTarget();
	void UnbindTarget();

private:
	// Presentation Runtime
	void ResetParryPresentationRuntime();

private:
	// View Synchronization
	void RefreshView();
	void RefreshActions();

private:
	// Event Handlers
	void HandlePlayerCombatResult(const FCombatSignalTargetPacket& Packet);
	void HandleTargetChanged(const FCombatTargetChange& Change);
	void HandleDeadStateChanged(EDeadState Previous, EDeadState Current);

	UFUNCTION()
	void HandlePlayerEndPlay(AActor* Actor, EEndPlayReason::Type Reason);

	UFUNCTION()
	void HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type Reason);
};
