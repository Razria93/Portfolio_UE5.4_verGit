#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Type/CMovementTypes.h"
#include "CPlayerController.generated.h"

UCLASS()
class PORTFOLIO_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACPlayerController();

private:
	// Components
	UPROPERTY(VisibleAnywhere)
	class UCPlayerFeedbackComponent* PlayerFeedbackComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCPlayerTargetSelectionComponent* PlayerTargetSelectionComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCTargetLockAssistComponent* TargetLockAssistComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCTargetHUDPresenterComponent* TargetHUDPresenterComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCDebugOverlayFocusComponent* DebugOverlayFocusComponent = nullptr;

	// Cached input
	FVector2D CachedMoveAxis2D = FVector2D::ZeroVector;
	bool bWalkInputHeld = false;
	bool bSprintInputHeld = false;
	EMovementRotationMode CachedMovementRotationMode = EMovementRotationMode::None;

public:
	// Component Query
	FORCEINLINE class UCPlayerTargetSelectionComponent* GetPlayerTargetSelectionComp() const { return PlayerTargetSelectionComponent; }

	// Debug Overlay Exec
	UFUNCTION(Exec)
	void DebugOverlaySelectNearestFocus();

	UFUNCTION(Exec)
	void DebugOverlaySelectOutlinerFocus(const FString& ActorName);

	UFUNCTION(Exec)
	void DebugOverlaySelectRecentCombatFocus();

	UFUNCTION(Exec)
	void DebugOverlaySelectPlayerTargetFocus();

	UFUNCTION(Exec)
	void DebugOverlayClearFocus();

protected:
	// Lifecycle
	virtual void PostInitializeComponents() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	// Look Input
	void InputLookYaw(float InAxisValue);
	void InputLookPitch(float InAxisValue);

	// Move Input
	void InputMoveForward(float InAxisValue);
	void InputMoveRight(float InAxisValue);

	// Movement Dispatch
	void FlushMoveInput();

	// Locomotion Input Dispatch
	void RefreshLocomotionGaitInput();
	EMovementRotationMode GetControlledPlayerMovementRotationMode() const;

	// Action Input
	void PressWalk();
	void ReleaseWalk();
	void PressSprint();
	void ReleaseSprint();

	void PressJump();
	void ReleaseJump();

	void PressSwordToggle();
	void PressComboAction();
	void PressGuard();
	void ReleaseGuard();
	void PressDodge();
	void PressExecution();

protected:
	// Player Target Selection
	void SynchronizeCombatTargetReferences();
	void ClearCombatTargetReferences();

	void PressTargetLock();
	void PressTargetSwitchLeft();
	void PressTargetSwitchRight();
};
