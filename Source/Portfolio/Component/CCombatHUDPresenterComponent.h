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
struct FCombatTargetChange;
enum class EDeadState : uint8;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PORTFOLIO_API UCCombatHUDPresenterComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCCombatHUDPresenterComponent();
	void SetControlledPlayer(ACPlayer* InPlayer);
	const FCombatHUDViewData& GetViewData() const { return ViewData; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD")
	TSubclassOf<UCCombatHUDWidget> WidgetClass;
	UPROPERTY(EditDefaultsOnly, Category="Combat HUD")
	int32 WidgetZOrder = 5;
	UPROPERTY(Transient)
	TObjectPtr<UCCombatHUDWidget> Widget;
	TWeakObjectPtr<ACPlayer> Player;
	TWeakObjectPtr<AActor> Target;
	TWeakObjectPtr<UCHealthComponent> PlayerHealth;
	TWeakObjectPtr<UCHealthComponent> TargetHealth;
	TWeakObjectPtr<UCBalanceComponent> TargetBalance;
	TWeakObjectPtr<UCCombatTargetComponent> CombatTarget;
	FCombatHUDViewData ViewData;
	bool bEndingPlay = false;

	void RefreshView();
	void RefreshTarget();
	void UnbindTarget();
	void HandleTargetChanged(const FCombatTargetChange& Change);
	void HandleDeadStateChanged(EDeadState Previous, EDeadState Current);
	UFUNCTION()
	void HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type Reason);
	UFUNCTION()
	void HandlePlayerEndPlay(AActor* Actor, EEndPlayReason::Type Reason);
};
