#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Type/CTargetingTypes.h"
#include "CTargetHUDPresenterComponent.generated.h"

class ACEnemy;
class APlayerController;
class UCTargetHUDWidget;
class UCCombatTargetComponent;
struct FCombatTargetChange;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PORTFOLIO_API UCTargetHUDPresenterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCTargetHUDPresenterComponent();

private:
	// Presentation Config
	UPROPERTY(EditDefaultsOnly, Category = "Target HUD")
	TSubclassOf<UCTargetHUDWidget> TargetHUDWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Target HUD")
	FTargetMarkerTuning TargetMarkerTuning;

private:
	// Component Reference
	UPROPERTY(Transient)
	APlayerController* OwnerPlayerController_Injected = nullptr;

	UPROPERTY(Transient)
	UCCombatTargetComponent* CombatTargetComponent_Injected = nullptr;

private:
	// Runtime Widget
	UPROPERTY(Transient)
	UCTargetHUDWidget* TargetHUDWidget = nullptr;

public:
	// Component Reference
	void InitializeReferences(APlayerController* InOwnerPlayerController);
	void SetCombatTargetComponent(UCCombatTargetComponent* InCombatTargetComponent);

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Validation
	bool ValidateRequiredReferences() const;

private:
	// Widget Lifecycle
	void CreateTargetHUDWidget();
	void DestroyTargetHUDWidget();

private:
	// Target State
	void HandleCombatTargetChanged(const FCombatTargetChange& InChange);
	void SynchronizeTargetState();

private:
	// Marker Presentation
	void UpdateTargetMarker();
	void HideTargetMarker();
	bool TryBuildTargetMarkerViewData(const ACEnemy* InTarget, FTargetMarkerViewData& OutViewData) const;
};
